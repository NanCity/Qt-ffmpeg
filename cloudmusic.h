#ifndef CLOUDMUSIC_H
#define CLOUDMUSIC_H
#include <QWidget>
class Config;
class Base;
class QNetworkRequest;
class QNetworkReply;
class QNetworkAccessManager;
class QNetworkCookie;
class QJsonObject;
class M_Tag;
class QMenu;
class QAction;
namespace Ui {
class CloudMusic;
}

struct cloudlist {
  int songid;
  int artid;
  int albumid;
  int lyricId;
  QString fileSize;
  QString album;
  QString songName;
  QString artist;
  QString picUrl;
  QString format;
  QString addTime;
  cloudlist()
      : songid{}, artid{}, fileSize{}, albumid{}, lyricId{}, album{},
        songName{}, artist{}, picUrl{}, addTime{}, format{} {};
};

class CloudMusic : public QWidget {
  Q_OBJECT

public:
  explicit CloudMusic(QWidget *parent = nullptr);
  ~CloudMusic();
  void InitHorizontalHead();
  void InitMenu();
  void RequestResources(QNetworkAccessManager *manger);
  void ParseJson(QJsonObject &rootobj);
  void DeleteSong();
  void LoadMyCloud();

private slots:
  void on_lineEdit_textChanged(const QString &text);
  void on_replyFinished(QNetworkReply *reply);
  void on_tableWidget_customContextMenuRequested(const QPoint &pos);

signals:
  //播放云盘歌曲
  void playAll();
  void play(CloudMusic *,const int id);
  void nextplay(const int id);
  

private:
  Ui::CloudMusic *ui;
  QMenu *menu;
  QAction *ActPlay;
  QAction *ActNextPlay;
  QAction *ActDownload;
  QAction *ActDelete;

  Base *base;
  Config *config;
  // M_Tag &tag = M_Tag::GetInstance();
  cloudlist cloundtag;

  QList<cloudlist> clist{};
  QNetworkRequest *request;
  QNetworkAccessManager *Delmang;
  QNetworkAccessManager *Manger;
  QList<QNetworkCookie> cookies;
};

#endif // CLOUDMUSIC_H
