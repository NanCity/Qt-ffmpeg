#ifndef RECOMMEND_H
#define RECOMMEND_H
//个性推荐Ui
#include "recommendeddaily.h"
#include "soloalbum.h"
#include <QPixmap>
#include <QWidget>
class Config;
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

struct RecPlaylist
{
	int id{};
	long long playCount{};
};


//首页UI
class Recommend : public QWidget {
	Q_OBJECT

public:
	explicit Recommend(QWidget* parent = nullptr);
	virtual ~Recommend();
	void InitLabel();
	void LoadPic();
	void setPic(const int _index = 0);
	void getPic();
	void NextPic();
	void Prevpic();
	//将所有的Btn_rec_..控件集合
	void addBtn_rec_();
	void addLab_rec_();
	//首页推荐歌单
	void RecommendPlaylist();
	SoloAlbum* getAlbumUi();
	RecommendedDaily* getRecDailyUi();

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event);
protected slots:
	//每日推荐单曲
	void on_btn_rec_1_clicked();

	void on_BannerReplyFinished(QNetworkReply*);
	void on_GetBannerPic(QNetworkReply*);
	void on_FinshedNewSong(QNetworkReply*);
	void on_FinshedNewDisc(QNetworkReply*);
	void on_FinshedGetAlubPic(QNetworkReply*);
	void on_FinshedNetRecommend(QNetworkReply*);
	//首页推荐歌单
	void on_FinishedNetRecPlaylist(QNetworkReply*);

private:
	unsigned int index{};
	Ui::Recommend* ui;
	Config* config;
	QMutex* mutex;
	Target target;
	QTimer* time;
	M_Tag* m_tag;
	//专辑UI
	SoloAlbum* soloalbum;
	//每日推荐UI
	RecommendedDaily* recDaily;

	QList<QPixmap> pixmap{};
	QList<Target> targetlist;
	QList<QLabel*> lablist{};
	QList<QLabel*> lab_title;

	QList<QPushButton*> btn_recAll{};
	QList<QLabel*>	lab_recAll;
	QNetworkRequest* request;
	QNetworkAccessManager* NetMangBanner;
	QNetworkAccessManager* NetGetBanner;
	//搜索banner内的资源
	QNetworkAccessManager* NetNewSong;
	QNetworkAccessManager* NetNewDisc;
	//专辑封面
	QNetworkAccessManager* NetAlbumPic;
	//推荐单曲列表
	QNetworkAccessManager* NetRecommend;
	//每日首页推荐歌单
	QNetworkAccessManager* NetRecPlaylist;
};

#endif // RECOMMEND_H
