#ifndef RECOMMEND_H
#define RECOMMEND_H
//个性推荐Ui

//每日推荐
#include "recommendeddaily.h"
//专辑
#include "soloalbum.h"


#include <QPixmap>
#include <QWidget>
class Config;
class M_Tag;
class QLabel;
class QMutex;
class QTimer;
class SongMenu;
class PhotoWall;
class QByteArray;
class QEventLoop;
class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;

namespace Ui {
	class Recommend;
}

//轮播图区分专辑和单曲
//struct Target {
//	int targetId;
//	QString typeTitle;
//	QString picUrl;
//	Target() : targetId{}, typeTitle{}, picUrl{} {}
//};

struct RecPlaylist
{
	int trackCount{};
	long long id{};
	long long playCount{};
};


//首页UI
class Recommend : public QWidget {
	Q_OBJECT

public:
	explicit Recommend(QWidget* parent = nullptr);
	virtual ~Recommend();
	//将所有的Btn_rec_..控件集合
	void addBtn_rec_();
	void addLab_rec_();
	//首页推荐歌单
	void RecommendPlaylist();
	//void getMyLikeMuiscId();
	SoloAlbum* getAlbumUi();
	SongMenu* getSoungMenu();
	RecommendedDaily* getRecDailyUi();

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event);
protected slots:
	//每日推荐单曲
	void on_btn_rec_1_clicked();
	//其余9个btn按钮
	void on_btn_rec_2_clicked();
	void on_btn_rec_3_clicked();
	void on_btn_rec_4_clicked();
	void on_btn_rec_5_clicked();
	void on_btn_rec_6_clicked();
	void on_btn_rec_7_clicked();
	void on_btn_rec_8_clicked();
	void on_btn_rec_9_clicked();
	void on_btn_rec_10_clicked();


	void on_FinshedNewSong(QNetworkReply*);
	void on_FinshedNewDisc(QNetworkReply*);
	void on_FinshedGetAlubPic(QNetworkReply*);
	void on_FinshedNetRecommend(QNetworkReply*);
	//首页推荐歌单
	void on_FinishedNetRecPlaylist(QNetworkReply*);

	//void on_FinshedNetMyLikeMusicId(QNetworkReply*);

private:
	unsigned int index{};
	//保存的图片顺序
	int Num;
	Ui::Recommend* ui;
	Config* config;
	QMutex* mutex;
	M_Tag* m_tag;
	//照片墙
	PhotoWall* photowall;
	//专辑UI
	SoloAlbum* soloalbum;
	//每日推荐UI
	RecommendedDaily* recDaily;
	//歌单UI
	SongMenu* songmuen;

	QList<QPixmap> pixmap{};
	QList<QLabel*> lablist{};
	QList<QLabel*> lab_title{};
	QList<QPushButton*> btn_recAll{};
	QList<QLabel*>	lab_recAll;

	//保存歌单ID和歌曲数量
	QList<RecPlaylist>RecList;
	QNetworkRequest* request;

	QNetworkAccessManager* NetNewSong;
	QNetworkAccessManager* NetNewDisc;
	//专辑封面
	QNetworkAccessManager* NetAlbumPic;
	//推荐单曲列表
	QNetworkAccessManager* NetRecommend;
	//每日首页推荐歌单
	QNetworkAccessManager* NetRecPlaylist;
	//我喜欢的音乐 -- ID
	//QNetworkAccessManager* NetMyLikeMusicId;
};

#endif // RECOMMEND_H
