﻿#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "discovrmusic.h"
#include "recommend.h"
#include "soloalbum.h"
#include <QGridLayout>
#include "base.h"
Dicovermusic::Dicovermusic(QWidget* parent) {
	this->setParent(parent);
	this->setObjectName("Dicovermusic");
	grid = new QGridLayout(this);
	recommend = new Recommend(this);
	RecPlayList = new RecommendPlaylist(this);
	newMusic = new NewMusicSudi(this);

	connect(this, &QTabWidget::tabBarClicked, this, &Dicovermusic::on_tabBarClicked);
	connect(RecPlayList, &RecommendPlaylist::getSongMenu, 
		recommend->getSoungMenu(), &SongMenu::getSongMenuID);


	InitTabWidget();
}
Dicovermusic::~Dicovermusic() {}

void Dicovermusic::InitTabWidget() {
	//soloAlbum = recommend->getAlbumUi();
	//添加各个子窗口
	this->addTab(recommend, "个性推荐");
	this->addTab(new QWidget(), "专属定制");
	this->addTab(RecPlayList, "歌单");
	this->addTab(new QWidget(), "排行榜");
	this->addTab(new QWidget(), "歌手");
	this->addTab(newMusic, "最新音乐");
	//设置样式
	this->setStyleSheet(
		"QTabBar::tab {background: #00000000; height:30px; "
		"width:60px;  font-weight:bold;font-size: 13px; color:red;"
		"border: none;"
		"min-width: 30px;margin-right: 30px;"
		"padding-left:20px;padding-right:20px;padding-top:5px;padding-bottom:5px;"
		"color:#686a6e;}"
		"QTabBar::tab:hover {background: white;}"
		"QTabBar::tab:selected {border-color: "
		"#3a3a3f;color:black;/*选中后显示下划线*/ border-bottom-color: "
		"#dcdde4;border-bottom: 3px solid #3c3e42;}");
}

SoloAlbum* Dicovermusic::getSoloAlbumUi() {
	return recommend->getAlbumUi();
}

RecommendedDaily* Dicovermusic::getRecDailyUi() {
	return recommend->getRecDailyUi();
}

SongMenu* Dicovermusic::getSongMuen()
{
	return recommend->getSoungMenu();
}

void Dicovermusic::on_tabBarClicked(int index)
{
	switch (index)
	{
	case  0:
	case 1:
		break;
	case 2:
		//获取推荐歌单
		RecPlayList->getRecPlayList();
		break;
	case 3:break;
	case 4:break;
	case 5:break;
	case 6:break;
	default:
		break;
	}
}
