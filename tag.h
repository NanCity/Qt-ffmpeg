#ifndef _M_TAG_H
#define _M_TAG_H
#include "config.h"
#include <QDialog>
#include <QList>
#include <QMap>
#include <QNetworkCookie>
#include <QPixmap>
#include <QString>
class QEventLoop;
class QNetworkReply;
class QJsonObject;
class Config;
class QNetworkCookie;
class QNetworkAccessManager;
class QNetworkRequest;

struct State {
  bool success;
  QString message;
  State() : success{}, message{} {}
};

class M_Tag : public QDialog {
  Q_OBJECT
public:
  M_Tag(QWidget *parent = nullptr);
  virtual ~M_Tag();
  QString GetArtist();
  QString GetTitle();
  QString GetAblue();
  QString GetSize();
  QString GetDuration();
  QPixmap GetAblueArt();
  void insertUrl(const int index, const int id);
  void SetArtist(QString art) { Artist = art; }
  void SetTitle(QString titel) { Title = titel; }
  void SetAblue(QString ablue) { Ablue = ablue; }
  void SetSize(QString _size) { Size = _size; }
  void SetDuration(QString dur) { Duration = dur; }
  void SetAblueArt(QPixmap ico) { AblueArt = ico; }
  void SetSongId(const QList<int> &id_list);
  void GetDetailsSong(const int id);
  void SetRequestHeader(QNetworkRequest *);
  void ParseDetailsSong(QJsonObject &root);
  //检查歌曲是否有版权
  void CheekState(const int index);
  State GetState() { return state; }
private slots:
  void on_replyFinshed(QNetworkReply *reply);
  void on_GetAblueArt(QNetworkReply *reply);
signals:
  void parseOk();
  void NoCopyright();

private:
  State state{};
  QString Artist{};
  QString Title{};
  QString Ablue{};
  QString Size{};
  QString Duration{};
  QPixmap AblueArt{};
  //音乐ID列表
  QList<int> id;
  Config conf{};
  QEventLoop *eventloop;
  QNetworkRequest *Request;
  QNetworkAccessManager *NetManager;
  QNetworkAccessManager *NetPase;
  //检查歌曲是否有版权
  QNetworkAccessManager *NetCheek;
  QList<QNetworkCookie> cookies;
};
static M_Tag gettag(M_Tag *);
#endif // _M_TAG_H
