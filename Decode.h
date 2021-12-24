
#ifndef DECODE_H_
#define DECODE_H_

#include <QAudioFormat>
#include <QAudioOutput>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QUrl>
/*
 * 导入外部 FFmpeg 库头文件
 */

#define _STDC_CONSTANT_MACROS

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

//方便调试，不需要打印东西的时候设置为0
#define DEBUG 1

//打印key value的之4
#define MESSAGE(Artis, value, format)                                          \
  qDebug() << "Key = " << key << " Value = " << value                          \
           << " format = " << format << endl;

/*
 * 使用FFmpeg库来对mp3等音频文件进行解析
 * Artist     --- 艺术家
 * Title      --- 标题(歌曲名称)
 * Ablue      --- 专辑
 * Size       --- 大小
 * Duration   --- 时长
 */
class Mp3tag {
private:
  Mp3tag() : Artis{}, Title{}, Ablue{}, Size{}, Duration{}, Picture{} {}

public:
  QString Artis;
  QString Title;
  QString Ablue;
  QString Size;
  QString Duration;
  QImage Picture;
  class cgMp3tag {
  public:
    ~cgMp3tag() {
      delete Mp3tag::tag;
      Mp3tag::tag = nullptr;
    }
  };
  ~Mp3tag() {}
  static Mp3tag *tag;
  static Mp3tag *init() {
    {
      if (tag == nullptr) {
        tag = new Mp3tag();
        static cgMp3tag CG; //回收垃圾
      }
      return tag;
    }
  }
};

//音频解码
class AudioDeCode : public QThread {
  Q_OBJECT
private:
  int seekMs{};
  QString url{};
  Mp3tag *tag = Mp3tag::init();
  /*AVFormatContext 结构体中有一个属性是metadata,
  我们在读取一个多媒体文件的时候，可以通过AVDictionaryEntry访问这个属性的数据
  */ 
  AVFormatContext *M_Format = NULL;
  AVFormatContext *pFmtCtx = NULL;
  //保存mp3-tag标签
  AVDictionaryEntry *Tag = NULL;
  //查询是音频流还是视频流
  int audioindex{-1};
signals:  
  void ERROR(QString str);
  void duration(int, int);
  void seekOk();   //处理控制,判断是否需要停止
  void nextsong(); //自动播放下一首

protected:
  void run();

public: //状态检查
    enum class controlType {
        control_none,
        control_stop,   //停止
        control_pause,  //暂停
        control_resume, //恢复
        control_play,   //播放
        control_seek,   //滑动
    };
  controlType type;
  QAudioOutput *audio;
  explicit AudioDeCode();
  virtual  ~AudioDeCode();
  QStringList DeCodeTag(const char *filename);
  //QStringList DeCodeTag(const AVDictionaryEntry *tag);
  QImage Image();
  Mp3tag *GetTag() { return tag; };
  //获取mp3进度
  QString Duration();
  void play(const QString &url);
  void stop();
  void pause();
  void resume();
  void seek(int value);
  bool runIsBreak();
  void runPlay();
  bool initAudio(int SampleRate);
  void debugErr(QString str, int err);
};

#endif // DECODE_H_
