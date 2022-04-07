#ifndef RECOMMENDPLAYLIST_H
#define RECOMMENDPLAYLIST_H

#include <QWidget>
#include <QMap>
#include <thread>
class QMutex;
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
	void addPlaylsit(QWidget*, const QPixmap);
	//精品歌单添加 btn
	void addBoutPlaylist(const QPixmap&);
	//获取歌单列表
	void getRecPlayList();
	//获取精品歌单
	void getBoutPlaylist(const QString& str = "全部歌单");
	void paseJosn(QJsonObject& obj, const QString& str, QList<Playlists>&);
	//解析精品歌单数据
	void paseBoutiquePlaylistJson(const QJsonObject& obj);
protected slots:
	//获取歌单数据
	void on_btnclicked();
	//精品歌单被点击
	void on_btn_clicked();
	//精品歌单
	void on_btn_Boutique_playlist_clicked();
	void on_finsgedNetGetReclist();
	void on_finshedNetGetPic();
	//获取精品歌单
	void on_finshedNetBoutPlaylist();
	//获取推荐歌单图片、
	void on_finshedNetCoverImg();
	void on_finshedNetBoutImage();
signals:
	void getSongMenu(const size_t id, const int limit);

private:
	Ui::RecommendPlaylist* ui;
	std::thread th{};
	std::thread botuiqueThread{};
	bool first{ true };

	// 0 == 推荐歌单，1 == 精品歌单
	int index{ 0 };
	// 精品歌单btn按钮ID
	int btn_clicked_id{0};
	//精品歌单UI栅格布局
	QGridLayout *scrollAreaWidgetContents_3_Grid;

	//歌单UI的顶部(精品歌单入口)
	QLabel* Bout_pic;
	QPushButton* btn_2;
	QLabel* lab;


	//推荐歌单主界面布局
	QGridLayout* playlistGrid;
	QList<Playlists> playlists{};
	//保存精品歌单数据
	QList<Playlists> bout_playlist{};
	//QList<QPushButton*> btnlist{};
	//按钮和推荐歌单ID对应关系
	QMap<QString, Playlists> btn_id{};
	//按钮和精品歌单ID对应关系
	QMap<QString, Playlists>btn_botuique_id;
	QNetworkAccessManager *MangerRecPlaylist;
	QNetworkReply* NetGetReclist{};
	QNetworkReply* NetGetPic{};
	QNetworkReply* NetCoverImg{};
	//获取所有精品歌单
	QNetworkReply* NetBoutPlaylist{};
	//获取精品歌单的封面
	QNetworkReply* NetBoutImage{};
};

#endif // RECOMMENDPLAYLIST_H
