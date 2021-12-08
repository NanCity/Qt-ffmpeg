/*
 * @Author: your name
 * @Date: 2021-09-20 12:16:36
 * @LastEditTime: 2021-12-01 17:05:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Music\base.h
 */
#ifndef BASE_H
#define BASE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

#include "Decode.h"
#include <QAction>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDebug>
#include <QMenu>
#include <QMutex>
class QTableWidget;
int TableMenu(QTableWidget *table);
class Base {
public:
  explicit Base(QWidget *paren = nullptr);
  ~Base();
  
public:
  void InitMedia();
  void InitTableWidget(QTableWidget *tab);
  static void DelTableWidgetRow(QTableWidget *tab);
  static void SerachData(QTableWidget *table, QString search_data);

public:
};

//初始化QAudioFormat

namespace AudioPlayer {
// 使用QAudioOutput播放pcm文件
class Player : public QIODevice {
private:
  QMutex mutex{};
  int len_written; //记录已写入多少字节
private slots:
  void handleStateChanged(QAudio::State newState);

public:
  QByteArray data_pcm; //存放pcm数据
  QAudioOutput *AudioOutPut = nullptr;
  Player();
  virtual ~Player() {
    len_written = 0;
    qDebug() << "delete --------------- player\n";
  };

  QAudioFormat initAudio();
  void setData(QByteArray &data) { data_pcm = std::move(data); }
  inline void dataClear() { data_pcm.clear(); };
  qint64 readData(char *data, qint64 maxlen); //重新实现的虚函数
  qint64 writeData(const char *, qint64 len) {
    return len;
  }; //它是个纯虚函数， 不得不实现
};
}; // namespace AudioPlayer

#endif // BASE_H
