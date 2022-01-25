#ifndef SOLOALBUM_H
#define SOLOALBUM_H

#include <QWidget>
class Base;
class QPushButton;
class QMenu;
class QAction;
namespace Ui {
class SoloAlbum;
}
struct Albumtag {
  int Artist_id;
  int Album_id;
  int Song_id;
  QString Title;
  QString Artist;
  QString Album;
  QString Duration;
  QString resourceTitle;

  Albumtag()
      : Artist_id{}, Album_id{}, Song_id{}, Title{}, Artist{}, Album{},
        Duration{}, resourceTitle{} {}
};

class SoloAlbum : public QWidget {
  Q_OBJECT

public:
  explicit SoloAlbum(QWidget *parent = nullptr);
  virtual ~SoloAlbum();
  void Init();
  void InitMenu();
  //专辑描述
  void setDescription(const QString str);
  //发版时间
  void setPublishTime(const QString str);
  //专辑名称
  void setlab_title(const QString str);
  //歌手
  void setlab_Artist(const QString str);
  //专辑封面
  void setlab_AlubPic(QPixmap pix);
  QStringList GetPlayList();
  QList<int> GetPlaylistID();
  QList<Albumtag> getAlbumTag() { return Albtag; }
  void setAlbumtag(Albumtag &alb);
  void LoadData();

protected slots:
  void on_playlist_customContextMenuRequested(const QPoint &pos);
signals:
  void loadOk();
  void Alb_play(SoloAlbum *, const int index);
  void Alb_playAll(SoloAlbum *);
  void Alb_Nextplay(SoloAlbum *, const int index, const QString url);

private:
  Base *base;
  //菜单
  QMenu *menu;
  QAction *Actplay;
  QAction *Actnextplay;
  QAction *Actdownload;
  QPushButton *like{};
  QPushButton *down{};
  QList<Albumtag> Albtag{};
  Ui::SoloAlbum *ui;
};

#endif // SOLOALBUM_H
