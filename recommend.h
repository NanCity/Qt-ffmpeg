#ifndef RECOMMEND_H
#define RECOMMEND_H
//个性推荐Ui

//每日推荐
#include "recommendeddaily.h"
//专辑
#include "soloalbum.h"

#include <QQueue>
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


struct RecPlaylist
{
	int trackCount{};
	long long id{};
	long long playCount{};
	QString picUrl{};
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
	void on_FinshedNewDisc();
	void on_FinshedGetAlubPic();
	void on_FinshedNetRecommend();
	//首页推荐歌单
	void on_FinishedNetRecPlaylist();
	//首页9个推荐歌单图片
	void on_FinshedNetpic();


	//void on_FinshedNetMyLikeMusicId(QNetworkReply*);

private:
	unsigned int index{};
	Ui::Recommend* ui;
	Config* config;
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
	//播放量
	QList<QLabel*> lab_PlayCount{};
	//标签
	QList<QLabel*>	lab_title;
	QList<QPushButton*> btn_recAll{};

	//保存歌单ID和歌曲数量
	QList<RecPlaylist>RecList;
	QNetworkRequest* request;
	QNetworkAccessManager * manger;
	QNetworkReply* NetNewSong{};
	QNetworkReply* NetNewDisc{};
	//专辑封面
	QNetworkReply* NetAlbumPic{};
	//推荐单曲列表
	QNetworkReply* NetRecommend{};
	//每日首页推荐歌单
	QNetworkReply* NetRecPlaylist{};
	//个性推荐UI 9张图片
	QNetworkReply* Netpic{};
	//我喜欢的音乐 -- ID
	//QNetworkAccessManager* NetMyLikeMusicId;
};

#endif // RECOMMEND_H
