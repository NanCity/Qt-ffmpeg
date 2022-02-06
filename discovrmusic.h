#ifndef DISCOVERMUSIC_H_
#define DISCOVERMUSIC_H_

/*
 *	发现音乐
 */

#include <QTabWidget>
class Recommend;
class SoloAlbum;
class QGridLayout;
class RecommendedDaily;
//发现音乐
class Dicovermusic : public QTabWidget {
  Q_OBJECT
public:
  Dicovermusic(QWidget *parent = nullptr);
  virtual ~Dicovermusic();
  void InitTabWidget();
  //个性推荐
  Recommend *getRecommendUi() { return recommend; }
  //专辑
  SoloAlbum *getSoloAlbum();
  //每日推荐
  RecommendedDaily *getRecDaily();

private:
  Recommend *recommend;
  QGridLayout *grid;
};

#endif // DISCOVERMUSIC_H_
