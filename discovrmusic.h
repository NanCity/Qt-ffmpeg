#ifndef DISCOVERMUSIC_H_
#define DISCOVERMUSIC_H_

/*
 *	发现音乐
 */

#include <QTabWidget>
class Recommend;
class SoloAlbum;
class QGridLayout;
class Dicovermusic : public QTabWidget {
  Q_OBJECT
public:
  Dicovermusic(QWidget *parent = nullptr);
  ~Dicovermusic();
  void InitTabWidget();
  Recommend *getRecommendUi() { return recommend; }
  SoloAlbum *getSoloAlbum() { return soloAlbum; };

private:
  SoloAlbum *soloAlbum;
  Recommend *recommend;
  QGridLayout *grid;
};

#endif // DISCOVERMUSIC_H_
