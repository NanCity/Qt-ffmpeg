#ifndef RECOMMEND_H
#define RECOMMEND_H
//个性推荐Ui
#include "soloalbum.h"
#include <QPixmap>
#include <QWidget>
class M_Tag;
class QLabel;
class QMutex;
class QTimer;
class QByteArray;
class QEventLoop;
class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;

namespace Ui {
class Recommend;
}

//轮播图区分专辑和单曲
struct Target {
  int targetId;
  QString typeTitle;
  QString picUrl;
  Target() : targetId{}, typeTitle{}, picUrl{} {}
};

class Recommend : public QWidget {
  Q_OBJECT

public:
  explicit Recommend(QWidget *parent = nullptr);
  virtual ~Recommend();
  void InitLabel();
  void LoadPic();
  void setPic(const int _index = 0);
  void getPic();
  void NextPic();
  void Prevpic();
  SoloAlbum *getAlbumUi();

protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
protected slots:
  void on_BannerReplyFinished(QNetworkReply *);
  void on_GetBannerPic(QNetworkReply *);
  void on_FinshedNewSong(QNetworkReply *);
  void on_FinshedNewDisc(QNetworkReply *);
  void on_FinshedGetAlubPic(QNetworkReply *);

private:
  unsigned int index{};
  Ui::Recommend *ui;
  QMutex *mutex;
  Target target;
  QTimer *time;
  M_Tag *m_tag;
  SoloAlbum *soloalbum;
  QList<QPixmap> pixmap{};
  QList<Target> targetlist;
  QList<QLabel *> lablist{};
  QNetworkRequest *request;
  QNetworkAccessManager *NetMangBanner;
  QNetworkAccessManager *NetGetBanner;
  //搜索banner内的资源
  QNetworkAccessManager *NetNewSong;
  QNetworkAccessManager *NetNewDisc;

  //专辑封面
  QNetworkAccessManager *NetAlbumPic;
};

#endif // RECOMMEND_H
