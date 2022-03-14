#ifndef RECOMMENDPLAYLIST_H
#define RECOMMENDPLAYLIST_H

#include <QWidget>
#include <QMap>
class QNetworkAccessManager;
class QNetworkReply;
//歌单列表栅格布局
class QGridLayout;
class QPushButton;
class QJsonObject;
class QLabel;
class QScrollArea;
namespace Ui {
	class RecommendPlaylist;
}

struct Playlists {
	//是否有小V
	bool avatarDetail{ true };
	//歌曲数量
	int trackCount{};
	//专辑ID
	size_t id{};
	//播放次数
	size_t playCount{};
	//描述
	QString name{};
	//创建者
	QString nickname{};
	//封面链接
	QString coverImgUrl{};

};

class RecommendPlaylist : public QWidget
{
	Q_OBJECT

public:
	explicit RecommendPlaylist(QWidget* parent = nullptr);
	~RecommendPlaylist();
	//推荐歌单添加 btn 
	void addPlaylsit(QWidget*, const QPixmap&);
	//精品歌单添加 btn
	void addBoutPlaylist(QWidget*, const QPixmap&);
	//获取歌单列表
	void getRecPlayList();
	//获取精品歌单
	void getBoutPlaylist(const QString& str = "全部歌单");
	void paseJosn(QJsonObject& obj, const QString& str, QList<Playlists>&);
protected slots:
	void on_btnclicked();
	//精品歌单
	void on_btn_Boutique_playlist_clicked();
	void on_finsgedNetGetReclist(QNetworkReply* reply);
	void on_finshedNetGetPic(QNetworkReply* reply);
	//获取精品歌单
	void on_finshedNetBoutPlaylist(QNetworkReply* reply);

signals:
	void getSongMenu(const size_t id, const int limit);

private:
	Ui::RecommendPlaylist* ui;
	bool first{ true };
	// 0 == 推荐歌单，1 == 精品歌单
	int index{ 0 };
	QScrollArea* Bout_Area;
	QLabel* Bout_pic;
	QPushButton* btn_2;
	QLabel* lab;
	//精品歌单主界面布局
	QGridLayout* grid;
	//推荐歌单主界面布局
	QGridLayout* playlistGrid;
	QList<Playlists> playlists{};
	QList<Playlists> bout_playlist{};
	//QList<QPushButton*> btnlist{};
	//按钮和歌单ID对应关系
	QMap<QString, Playlists> btn_id{};
	QNetworkAccessManager* NetGetReclist;
	QNetworkAccessManager* NetGetPic;
	QNetworkAccessManager* NetBoutPlaylist;
};

#endif // RECOMMENDPLAYLIST_H
