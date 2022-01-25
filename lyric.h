#ifndef LYRIC_H
#define LYRIC_H

#include "qpixmap.h"
#include <QMouseEvent>
#include <QWidget>

class QVBoxLayout;

namespace Ui {
class lyric;
}

#define ERROR(string)                                                          \
  qDebug() << __FILE__ << ' ' << __LINE__ << ' ' << string << '\n';

//表示一行歌词（一个时间点+对应的歌词文本）
class LyricLine {
public:
  int time;
  QString text;
  LyricLine(int _time, QString _text) : time(_time), text(_text) {}
};

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QScrollBar;

class QMenu;
class QLabel;
class DesktopLyrics;

class lyric : public QWidget {
  Q_OBJECT
public:
  explicit lyric(QWidget *parent = nullptr);
  virtual ~lyric();

  DesktopLyrics *destlyric;

  void setMessage(QImage img, QString Art, QString title);
  void GetTheLyricsName(QString rhs);
  void ReadLyric(QString filepath);
  void setpic(QPixmap  pix);
  int GetID() { return id; }
  /**********************处理歌词************************/
  //将歌词文件的内容处理为歌词结构的QList
  bool process(QString content);
  //根据时间找到对应位置的歌词
  int getIndex(qint64 position);
  //显示当前播放进度的歌词
  void showcontent(qint64 position);
  //根据下标获得歌词内容
  QString getLyricText(int index);
  //清空显示歌词
  void clearlabel();
  //将所有的标签存入list
  void creatlab();
  //显示歌词
  void showlyric();
  //自动向下滑动
  void slidingDown( int index);
private slots:
  void on_ReplyFinished(QNetworkReply *reply);
signals:
  void lyricChanged();


private:
  int id{};
  QString name{};
  bool mHasTimer{};


  QList<QLabel*>listLab{};
  //储存所有歌词
  QList<LyricLine> lines{};
  QVBoxLayout *scrollVertical;
  QNetworkAccessManager *Netmanger;
  QNetworkAccessManager *Netmangelyric;
  QScrollBar *vScrollbar;
  Ui::lyric *ui;
};

#endif // LYRIC_H
