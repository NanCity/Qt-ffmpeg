﻿#include "base.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QTableWidget>
#include <QWheelEvent>
#include <QScrollBar>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMovie>
#include "Loading.h"

Base::Base(QTableWidget* parent) : tab{ parent } {

	tab->installEventFilter(this);
	this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	movie = new Loading(tab);
	InitTableWidget();
	//可以接受鼠标操作
	tab->setContextMenuPolicy(Qt::CustomContextMenu);
	//与鼠标右键关联
	connect(tab, &QTableWidget::customContextMenuRequested, this,
		&Base::RightClickMouse);

	menu = new QMenu(tab);
	CreatorMenu();
	NetMyLikeMusicId = new QNetworkAccessManager(this);
	connect(NetMyLikeMusicId, &QNetworkAccessManager::finished, this, &Base::on_FinshedNetMyLikeMusicId);

	//获取我喜欢的音乐ID
	GetMyLikeMusicID();
}

Base::~Base() {}

void Base::loadMovie()
{
	if (movie->isHidden()) {
		movie->show();
	}
	movie->start();
}

void Base::closeMovie()
{
	movie->Stop();
	movie->hide();
}

bool Base::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == tab) {
		if (event->type() == QEvent::Wheel) {
			QWheelEvent* wheel = static_cast<QWheelEvent*>(event);
			//y() < 0 鼠标滚轮向自己滑动
			if (wheel->angleDelta().y() < 0) {
				emit loadNextPage();
				fprintf(stdout, "loadNextPage\n");
			}
		}
	}
	return QTableWidget::eventFilter(obj, event);
}


//初始化TableWidget
void Base::InitTableWidget() {
	//设置无边框
	tab->setFrameShape(QFrame::NoFrame);
	//设置触发条件：不可编辑
	tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tab->setSortingEnabled(false); //启动排序
	// item 水平表头自适应大小
	tab->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tab->horizontalHeader()->setDefaultSectionSize(35);
	// item 垂直表头自适应大小
	// ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//是否使用交替的颜色绘制背景
	tab->setAlternatingRowColors(true);
	tab->setSelectionBehavior(
		QAbstractItemView::SelectRows); //设置选择行为时每次选择一行
}

void Base::DelTableWidgetRow() {
	for (int index = tab->rowCount(); index >= 0; --index) {
		tab->removeRow(index);
	}
}

void Base::SerachData(QString search_data) {
	const int curRow = tab->rowCount();
	if ("" == search_data) {
		for (int i = 0; i != curRow; i++) {
			tab->setRowHidden(i, false);
		}
	}
	else {
		//列出所有的条件的cell索引
		QList<QTableWidgetItem*> item =
			tab->findItems(search_data, Qt::MatchContains);
		for (int i = 0; i != curRow; i++) {
			//隐藏所有行
			tab->setRowHidden(i, true);
		}
		if (!item.isEmpty()) {
			//打印查询到的结果
			for (int i = 0; i != item.count(); i++) {
				tab->setRowHidden(item.at(i)->row(), false);
			}
		}
	}
}

void Base::InsertDataInfoTableWidget(const QStringList value, const int index) {
	tab->insertRow(index); //插入新的一行
	for (int row = 0; row != value.length(); ++row) {
		tab->setItem(index, row, new QTableWidgetItem(value.at(row)));
	}
}

//添加tablewidget小部件
QWidget* Base::setItemWidget(int statue) {
	QHBoxLayout* hbox = new QHBoxLayout(tab);
	QPushButton* like = new QPushButton(this);
	if (statue == 1) {
		like->setToolTip("取消喜欢");
		like->setIcon(QIcon(":/images/like.png"));
	}
	else
	{
		like->setToolTip("喜欢");
		like->setIcon(QIcon(":/images/btn_unlike_h.png"));
	}

	QPushButton* down = new QPushButton(this);
	down->setIcon(QIcon(":/images/btn_download_h.png"));
	down->setToolTip("下载");
	like->setMaximumSize(QSize(20, 25));
	down->setMaximumSize(QSize(20, 25));
	hbox->addWidget(like);
	hbox->addWidget(down);
	widget = new QWidget(this);

	//widget->setMinimumHeight(40);
	widget->setMaximumSize(60, 40);
	widget->setMinimumSize(60, 40);
	widget->setLayout(hbox);
	return widget;
}

//下载歌曲
void Base::Download(const unsigned int in) {}

void Base::CreatorMenu() {
	Actcomment = new QAction(QIcon(":/images/comment_h.png"), "评论", menu);
	Actplay = new QAction(QIcon(":/images/bottom/btn_play_h.png"), "播放", menu);
	Actnextplay = new QAction(QIcon(":/images/bottom/btn_single_h.png"),
		"下一首播放", menu);
	Actcollect =
		new QAction(QIcon(":/images/btn_openfile_h.png"), "收藏到歌单", menu);
	Actdownload = new QAction(QIcon(":/images/btn_download_h.png"), "下载", menu);

	menu->addAction(Actcomment);
	menu->addAction(Actplay);
	menu->addAction(Actnextplay);
	//加一条横线
	menu->addSeparator();
	menu->addAction(Actcollect);
	menu->addAction(Actdownload);
}

void Base::CreatorMenu(const QList<QAction*> Act) {
	menu->clear();
	menu->addActions(Act);
}

/*
* uid 传入的是用户ID(需要先登录)
*/
void Base::GetMyLikeMusicID()
{
	QString userid = config.GetValue("/Userinfo/userId");
	if (userid.isEmpty()) {
		QLabel* lab = new QLabel(this);
		lab->setText("还未登录账号");
		lab->move(this->width() / 2 + 10, 40);
		return;
	}
	else
	{
		QString Url{ QString("http://localhost:3000/likelist?uid=%1").arg(userid.toInt()) };
		QNetworkRequest* request{ config.setCookies() };
		request->setUrl(Url);
		NetMyLikeMusicId->get(*request);
	}
}

bool Base::isLike(const int ID)
{
	QList<int>::const_iterator it{};
	//std::find(STL算法)返回一个迭代器，如若没有找到则返回尾后迭代器
	it = std::find(likeMusicId.cbegin(), likeMusicId.cend(), ID);
	if (it != likeMusicId.cend()) {
		return true;
	}
	else
	{
		return false;
	}
}

//相应鼠标右键槽函数
void Base::RightClickMouse(const QPoint& pos) {
	if (tab->itemAt(pos) != nullptr) {
		menu->exec(QCursor::pos());
	}
}


void Base::on_FinshedNetMyLikeMusicId(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		//我喜欢的音乐了列表ID
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = document.object();
			QJsonValue idsAry = rot.value("ids");
			Config cfg("../config/MyLikeMusicId.ini");
			likeMusicId.clear();
			if (idsAry.isArray()) {
				QJsonArray ary = idsAry.toArray();
				int i = 0;
				int id{};
				cfg.SetBeginGroup("MusicId");
				foreach(const QJsonValue & rhs, ary) {
					id = rhs.toInt();
					cfg.SetValue(QString::number(i), QVariant(id));
					likeMusicId.push_back(id);
					++i;
				}
				cfg.endGroup();
			}
		}
		else
		{
			fprintf(stdout, "Json format Error\n");
		}
	}
	reply->deleteLater();
}







/*******************初始化QAudioOutPut***********************/
// Player::Player() : data_pcm(0) {
//  this->open(QIODevice::ReadOnly); // 为了解决QIODevice::read (QIODevice):
//                                   // device not open.
//  initAudio();
//  AudioOutPut = new QAudioOutput(initAudio(), this);
//  len_written = 0;
//  connect(AudioOutPut, &QAudioOutput::stateChanged, this,
//          &Player::handleStateChanged);
//}
//
// void Player::handleStateChanged(QAudio::State state) {
//  switch (state) {
//  case QAudio::ActiveState:
//    qDebug() << "playing....... \n";
//    break;
//  case QAudio::SuspendedState:
//    //暂停
//    qDebug() << "suspend....... \n";
//    break;
//  case QAudio::StoppedState:
//    qDebug() << "************Call Stop()***********\n";
//    break;
//  case QAudio::IdleState:
//    //传入的数据是空的，音频系统的缓冲区是空的
//    qDebug() << "error: pcm_data is empty:\n";
//    break;
//  case QAudio::InterruptedState:
//    qDebug() << "Higher priority entry\n";
//    dataClear();
//    break;
//  default:
//    break;
//  }
//}
//
// qint64 Player::readData(char *data, qint64 maxlen) {
//  if (len_written >= data_pcm.size())
//    return 0;
//  int len = 0;
//
//  //计算未播放的数据的长度.
//  len = (len_written + maxlen) > data_pcm.size()
//            ? (data_pcm.size() - len_written)
//            : maxlen;
//  memcpy(data, data_pcm.data() + len_written,
//         len);        //把要播放的pcm数据存入声卡缓冲区里.
//  len_written += len; //更新已播放的数据长度.
//  return len;
//}
//
// QAudioFormat format() {
//  QAudioFormat fmt;
//  fmt.setSampleRate(44100);
//  fmt.setChannelCount(2);
//  fmt.setSampleSize(16);
//  fmt.setCodec("audio/pcm");
//  fmt.setByteOrder(QAudioFormat::LittleEndian);
//  fmt.setSampleType(QAudioFormat::SignedInt);
//  QAudioDeviceInfo info(
//      QAudioDeviceInfo::defaultOutputDevice()); //选择默认输出设备
//  if (!info.isFormatSupported(fmt)) {
//    qDebug() << "The output device does not support this format and cannot "
//                "play audio";
//  }
//  return fmt;
//}
//
// QAudioFormat Player::initAudio() {
//  QAudioFormat fmt{};
//  fmt.setSampleRate(44100);
//  fmt.setChannelCount(2);
//  fmt.setSampleSize(16);
//  fmt.setCodec("audio/pcm");
//  fmt.setByteOrder(QAudioFormat::LittleEndian);
//  // QAudioOutput::setVolume设置声音大小之后，声音会出现噪音
//  //解决办法是setSampleType由UnSignedInt改为SignedInt
//  fmt.setSampleType(QAudioFormat::SignedInt);
//  QAudioDeviceInfo info(
//      QAudioDeviceInfo::defaultOutputDevice()); //选择默认输出设备
//  if (!info.isFormatSupported(fmt)) {
//    qDebug() << "The output device does not support this format and cannot "
//                "play audio\n";
//  }
//  return fmt;
//}
