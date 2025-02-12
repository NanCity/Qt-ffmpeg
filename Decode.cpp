﻿#include "Decode.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QMutex>
#include <QDialog>
// Mp3tag *Mp3tag::tag = nullptr;

AudioDeCode::AudioDeCode() {
	audio = nullptr;
	tag = new M_Tag();
	mutex = new QMutex();
	//注册所有的解码器格式
	// av_register_all(); 已被弃用
	//初始化网络库 （可以打开rtsp rtmp http协议的流媒体视频）
	avformat_network_init();
	type = controlType::control_none;
}
AudioDeCode::~AudioDeCode() {
	delete tag;
	tag = nullptr;
	delete mutex;
	mutex = nullptr;
}

bool AudioDeCode::initAudio(int SampleRate) {
	QAudioFormat format;
	if (audio != NULL)
		return true;

	format.setSampleRate(SampleRate);                //设置采样率
	format.setChannelCount(2);                       //设置通道数
	format.setSampleSize(16);                        //样本数据16位
	format.setCodec("audio/pcm");                    //播出格式为pcm格式
	format.setByteOrder(QAudioFormat::LittleEndian); //默认小端模式
	format.setSampleType(QAudioFormat::SignedInt);   //无符号整形数
	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(format)) {
		qDebug() << "输出设备不支持该格式，不能播放音频";
		return false;
	}
	audio = new QAudioOutput(format, this);
	audio->setVolume(80 / 100.0);
	audio->setBufferSize(100000);
	return true;
}

void AudioDeCode::play(const QString& url) {

	 bool ok{ false };
	ok = url.toInt(&ok);
	if (ok == true) {
		const int id = url.toInt();
		//获取mp3标签
		tag->GetDetailsSong(id);
		//判断是否可以播放
		//if (tag->GetState().success == false) {
		//	QDialog dialog;
		//	if (dialog.exec() == QDialog::Accepted) {
		//		dialog.setWindowTitle(tag->GetState().message);
		//		emit nextsong();
		//	}
		//	else
		//	{
		//		stop();
		//	}
		//	return;
		//}
		QString Url{ "https://music.163.com/song/media/outer/url?id=" + url };
		this->url = Url;
	}
	else
	{
		this->url = url;
		//解析本地mp3文件
		DeCodeTag(url.toLocal8Bit().data());
	}

	//在这里判断一下，是否为https链接，是的话用tag解析，否则调用DeCodeTag本地解析
	// Qt::CaseSensitive找到返回 true
	//if (url.startsWith("https", Qt::CaseSensitive)) {
	//  tag->CheekState(id);
	//  tag->GetDetailsSong(id);
	//} else {
	//  //解析本地mp3文件
	//  DeCodeTag(url.toLocal8Bit().data());
	//}

	type = controlType::control_play;
	if (!this->isRunning()) {
		this->start();
	}
}

void AudioDeCode::stop() {
	if (this->isRunning()) {
		type = controlType::control_stop;
	}
}

void AudioDeCode::pause() {
	if (this->isRunning()) {
		type = controlType::control_pause;
	}
}

void AudioDeCode::resume() {
	if (this->isRunning()) {
		type = controlType::control_resume;
	}
}

void AudioDeCode::run() {
	if (!initAudio(44100)) {
		emit ERROR("输出设备不支持该格式，不能播放音频");
	}
	while (true) {
		switch (type) {
		case controlType::control_none:
			msleep(100);
			break;
		case controlType::control_play:
			type = controlType::control_none;
			runPlay();
			break; //播放
		default:
			type = controlType::control_none;
			break;
		}
	}
}

void AudioDeCode::seek(int value) {
	if (this->isRunning()) {
		seekMs = value;
		type = controlType::control_seek;
	}
}

void AudioDeCode::debugErr(QString str, int err) {
	char errbuf[512] = { 0 };
	av_strerror(err, errbuf, sizeof(errbuf));
	qDebug() << str << ":" << errbuf;
	emit ERROR(str + ":" + errbuf);
}

bool AudioDeCode::runIsBreak() {
	bool ret = false;
	//处理播放暂停
	if (type == controlType::control_pause) {
		while (type == controlType::control_pause) {
			audio->suspend();
			msleep(500);
		}

		if (type == controlType::control_resume) {
			audio->resume();
		}
	}

	if (type == controlType::control_play) //重新播放
	{
		ret = true;
		if (audio->state() == QAudio::ActiveState)
			audio->stop();
	}

	if (type == controlType::control_stop) //停止
	{
		ret = true;
		if (audio->state() == QAudio::ActiveState)
			audio->stop();
	}
	return ret;
}

//开始解码播放
void AudioDeCode::runPlay() {
	int ret{};
	int destMs{}, currentMs{};

	if (audio == NULL) {
		emit ERROR("输出设备不支持该格式，不能播放音频");
		return;
	}

	AVFormatContext* pFmtCtx = NULL;

	ret = avformat_open_input(
		&pFmtCtx, this->url.toLocal8Bit().data(), NULL,
		NULL); //打开音视频文件并创建AVFormatContext结构体以及初始化.
	if (ret != 0) {
		debugErr("avformat_open_input", ret);
		return;
	}

	ret = avformat_find_stream_info(pFmtCtx, NULL); //初始化流信息
	if (ret != 0) {
		debugErr("avformat_find_stream_info", ret);
		return;
	}

	audioindex =
		av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audioindex == AVERROR_STREAM_NOT_FOUND) {
		qDebug() << __FILE__ << " " << __LINE__ << "\tStream not found\n";
		return;
	}
	qDebug() << "audioindex:" << audioindex;

	//查询解码器
	AVCodec* acodec = avcodec_find_decoder(
		pFmtCtx->streams[audioindex]->codecpar->codec_id); //获取codec
	AVCodecContext* acodecCtx = avcodec_alloc_context3(
		acodec); //构造AVCodecContext ,并将vcodec填入AVCodecContext中
	avcodec_parameters_to_context(
		acodecCtx,
		pFmtCtx->streams[audioindex]->codecpar); //初始化AVCodecContext

	ret = avcodec_open2(
		acodecCtx, NULL,
		NULL); //打开解码器,由于之前调用avcodec_alloc_context3(vcodec)初始化了vc,那么codec(第2个参数)可以填NULL
	if (ret != 0) {
		debugErr("avcodec_open2", ret);
		return;
	}
	SwrContext* swrctx = NULL;
	swrctx = swr_alloc_set_opts(swrctx, av_get_default_channel_layout(2),
		AV_SAMPLE_FMT_S16, 44100,
		acodecCtx->channel_layout, acodecCtx->sample_fmt,
		acodecCtx->sample_rate, NULL, NULL);
	swr_init(swrctx);

	destMs = av_q2d(pFmtCtx->streams[audioindex]->time_base) * 1000 *
		pFmtCtx->streams[audioindex]->duration;
#ifdef _DEBUG
	qDebug() << "码率:" << acodecCtx->bit_rate;
	qDebug() << "格式:" << acodecCtx->sample_fmt;
	qDebug() << "通道:" << acodecCtx->channels;
	qDebug() << "采样率:" << acodecCtx->sample_rate;
	qDebug() << "时长:" << destMs;
	qDebug() << "解码器:" << acodec->name;
#endif // _DEBUG

	int outChannelCount = acodecCtx->channels;
	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	audio->stop();
	QIODevice* io = audio->start();
	// 设置音频缓冲区间 16bit   44100  PCM数据, 双声道
	uint8_t* out_buffer = (uint8_t*)av_malloc(2 * 44100);
	while (1) {
		if (runIsBreak())
			break;

		if (type == controlType::control_seek) {
			//跳转到指定的播放位置
			av_seek_frame(pFmtCtx, audioindex,
				seekMs / (double)1000 /
				av_q2d(pFmtCtx->streams[audioindex]->time_base),
				AVSEEK_FLAG_BACKWARD);
			type = controlType::control_none;
			emit seekOk();
		}

		ret = av_read_frame(pFmtCtx, packet);
		if (ret != 0) {
			debugErr("av_read_frame", ret);
			emit duration(destMs, destMs);
			//自动播放一下一首
			emit nextsong();
			break;
		}

		if (packet->stream_index == audioindex) {
			//解码一帧数据
			ret = avcodec_send_packet(acodecCtx, packet);
			av_packet_unref(packet);

			if (ret != 0) {
				debugErr("avcodec_send_packet", ret);
				continue;
			}
			while (avcodec_receive_frame(acodecCtx, frame) == 0) {
				if (runIsBreak())
					break;

				int len = swr_convert(swrctx, &out_buffer,
					2 * 44100, //输出
					(const uint8_t**)frame->data,
					frame->nb_samples); //输入
//将重采样后的data数据发送到输出设备,进行播放
				av_samples_get_buffer_size(nullptr, outChannelCount, frame->nb_samples,
					AV_SAMPLE_FMT_S16, 1);
				int out_buff_size =
					len * outChannelCount * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				while (audio->bytesFree() < out_buff_size) {
					if (runIsBreak())
						break;
					msleep(10);
				}
				if (!runIsBreak())
					io->write((const char*)out_buffer, out_buff_size);
				currentMs =
					av_q2d(pFmtCtx->streams[audioindex]->time_base) * 1000 * frame->pts;
				// qDebug()<<"时长:"<<destMs<<currentMs;
				emit duration(currentMs, destMs);
			}
		}
	}

	//释放内存
	av_frame_free(&frame);
	av_packet_free(&packet);
	swr_free(&swrctx);
	avcodec_free_context(&acodecCtx);
	avformat_close_input(&pFmtCtx);
}

QStringList AudioDeCode::DeCodeTag(const char* filename) {
	QStringList list{};
	qDebug() << "Curmp3: " << filename << '\n';
	M_Format = avformat_alloc_context();
	if (avformat_open_input(&M_Format, filename, NULL, NULL) != 0) {
		qDebug()<<"Can't open file.";
		return QStringList();
	}

	// 查找流信息，把它存入AVFormatContext中
	if (avformat_find_stream_info(M_Format, 0) < 0) {
		ERROR("Failed to obtain audio stream\n");
	}

	while (
		(Tag = av_dict_get(M_Format->metadata, "", Tag, AV_DICT_IGNORE_SUFFIX))) {
#ifdef __LINUX__ // Linux 操作系统的API
		/*  #include <strings.h>  */
		if (!strcasecmp(Tag->key, "title") || !strcasecmp(Tag->key, "artist") ||
			!strcasecmp(Tag->key, "album")) {
			MESSAGE(Tag->key, Tag->value, Format->iformat->name);
			list.append(Tag->value);
		}
#elif WIN32 // windows 操作系统的API
		if (!_stricmp(Tag->key, "title")) {
			// tag->Title = Tag->value;
			tag->SetTitle(Tag->value);
		}
		else if (!_stricmp(Tag->key, "artist")) {
			// tag->Artis = Tag->value;
			tag->SetArtist(Tag->value);
		}
		else if (!_stricmp(Tag->key, "album")) {
			// tag->Ablue = Tag->value;
			tag->SetAblue(Tag->value);
		}
		else {
			continue; //跳过一些无用tag信息
		}
		list.append(Tag->value);
	}
#endif
	//获取音频文件的大小
	QFile fp(filename);
	QString _size =
		QString("%0").number((double)fp.size() / 1024 / 1024, 'f', 2) + " MB";
	list.append(_size);
	fp.close();
	// tag->Size = _size;
	tag->SetSize(_size);
	// tag->Duration = Duration();
	tag->SetDuration(Duration());
	// tag->Picture = Image();
	tag->SetAblueArt(Image());
	list.append(Duration()); //获取音频时长
#ifdef _DEBUG
	/*  qDebug() << "Artis = " << tag->Artis << " Title = " << tag->Title
			   << " Ablue = " << tag->Ablue << " Size = " << tag->Size
			   << " Duration = " << tag->Duration << "\n";*/
#endif // _DEBUG
	avformat_close_input(&M_Format);
	avformat_free_context(M_Format);
	M_Format = nullptr;
	//本地音乐解码完毕
	emit LocalparseOk();
	return list;
}

QPixmap AudioDeCode::Image() {
	if (M_Format->iformat->read_header(M_Format) < 0) {
		printf("No header format");
	}
	for (unsigned int i = 0; i < M_Format->nb_streams; i++) {
		if (M_Format->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
			AVPacket pkt = M_Format->streams[i]->attached_pic;
			//使用QImage读取完整图片数据（注意，图片数据是为解析的文件数据，需要用QImage::fromdata来解析读取）
			QImage image = QImage::fromData((uchar*)pkt.data, pkt.size);
			// tag->Picture = QPixmap::fromImage(image);
			tag->SetAblueArt(QPixmap::fromImage(image));
			break;
		}
	}
	// return QPixmap();
	// return tag->Picture;
	return tag->GetAblueArt();
}

QString AudioDeCode::Duration() {
	unsigned int duration = 0, mins = 0, cesc = 0;
	duration = M_Format->duration;
	mins = (duration / 1000000) / 60; //计算分钟
	cesc = (duration / 1000000) % 60; //计算秒数
	return QString("%1 : %2")
		.arg(mins, 2, 10, QChar('0'))
		.arg(cesc, 2, 10, QChar('0'));
}