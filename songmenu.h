#ifndef SONGMENU_H
#define SONGMENU_H

#include <QWidget>
#include "config.h"
#include <QMap>

class Base;
class Temptag;
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
	//歌单名称
	QString name{};
	//歌单封面
	QString coverImgUrl{};
	QString createTime{};
	QPixmap Userpic{};
	QString nickname{};

};

class SongMenu : public QWidget
{
	Q_OBJECT
public:
	explicit SongMenu(QWidget* parent = nullptr);
	~SongMenu();
	void loadData();
	//获取我喜欢的歌曲列表ID,方便后续操作
	void RequestSongTable();
	//返回歌单名字
	QStringList getSongMenu();
	void RequestUserSongMenu();
	void SongMenuAt(const int index = 0);
	//拿到歌单之后，直接请求歌单中的歌曲
	void getSongMenuID(const long long ID = 0, const int limit = 10);
	//创建歌单
	void CreatorSongMuen(const QString &name);
	void DelereSongMuenu(const int ID);
signals:
	//加载歌单列表
	void DataLoading();
	void CreatorSongMenuOk();
	void DeleteOk();
protected slots:
	void on_finshedNetSongMenu(QNetworkReply*);
	void on_finsedNetAllSong(QNetworkReply*);
private:
	Ui::SongMenu* ui;
	int userId{};
	int songMenuID{};
	int curtableindex{};
	Base* base;
	Config config{};
	//保存用户歌单
	QList<UserSongMuen> songlistMenu{};
	QList<Temptag>taglsit{};
	QNetworkAccessManager* NetSongMenu;
	QNetworkAccessManager* NetAllSong;
};

#endif // SONGMENU_H
