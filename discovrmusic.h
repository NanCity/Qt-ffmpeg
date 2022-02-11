#ifndef DISCOVERMUSIC_H_
#define DISCOVERMUSIC_H_

/*
 *	发现音乐(首页)
 */

#include <QTabWidget>
 //歌单UI
#include "songmenu.h"
//个性推荐UI
#include "recommend.h"
//专辑
#include "soloalbum.h"


class SoloAlbum;
class QGridLayout;
class RecommendedDaily;
//发现音乐
class Dicovermusic : public QTabWidget {
	Q_OBJECT
public:
	Dicovermusic(QWidget* parent = nullptr);
	virtual ~Dicovermusic();
	void InitTabWidget();
	//个性推荐
	Recommend* getRecommendUi() { return recommend; }
	//专辑
	SoloAlbum* getSoloAlbumUi();
	//每日推荐
	RecommendedDaily* getRecDailyUi();
	//歌单
	SongMenu* getSongMuen();

private:
	Recommend* recommend;
	QGridLayout* grid;
};

#endif // DISCOVERMUSIC_H_
