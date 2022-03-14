#ifndef SONGMENU_H
#define SONGMENU_H

#include <QWidget>
#include "config.h"
#include <QMap>
#include "tag.h"

class Base;
class UserSongMuen;
class QNetworkReply;
class QNetworkAccessManager;

namespace Ui {
	class SongMenu;
}

//用户歌单
class UserSongMuen {
public:
	//歌单用户ID
	int userID{};
	//歌单ID
	long long id{};
	//歌曲数量
	int trackCount{};
	//收藏量
	size_t subscribedCount{};
	//播放量
	size_t playCount{};
	//歌单名称
	QString name{};
	//歌单封面
	QString coverImgUrl{};
	QString createTime{};
	QPixmap Userpic{};
	QString nickname{};
	//简介 
	QString description{};
	//标签
	QString tags{};
	//各大名创建者的图片
	QString avatarUrl{};
};

class SongMenu : public QWidget
{
	Q_OBJECT
public:
	explicit SongMenu(QWidget* parent = nullptr);
	virtual ~SongMenu();
	void loadData();
	//获取我喜欢的歌曲列表ID,方便后续操作
	void RequestSongTable();
	//返回歌单名字
	QStringList getSongMenu();
	void RequestUserSongMenu();
	void SongMenuAt(const int index = 0);
	//拿到歌单之后，直接请求歌单中的歌曲
	void getSongMenuID(const size_t ID = 0, const int limit = 10);
	//创建歌单
	void CreatorSongMuen(const QString& name);
	void DelereSongMuenu(const int ID);
signals:
	//加载歌单列表
	void DataLoading();
	void CreatorSongMenuOk();
	void DeleteOk();
protected slots:
	void on_finshedNetSongMenu(QNetworkReply*);
	void on_finsedNetAllSong(QNetworkReply*);
	void on_finshedNetDetail(QNetworkReply*);
	void on_finsedNetPic(QNetworkReply*);
private:
	Ui::SongMenu* ui;
	int userId{};
	int songMenuID{};
	int curtableindex{};
	Base* base;
	Config config{};
	UserSongMuen tempMuenInfo{};
	//保存用户歌单
	QList<UserSongMuen> songlistMenu{};

	M_Tag tag{};
	QList<Temptag>* taglsit;

	QNetworkAccessManager* NetSongMenu;
	QNetworkAccessManager* NetAllSong;
	//拿到歌单的描述之类的
	QNetworkAccessManager* NetDetail;
	QNetworkAccessManager* NetPic;
};

#endif // SONGMENU_H
