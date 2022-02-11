#ifndef SEARCH_H
#define SEARCH_H
#include <QMap>
#include <QWidget>
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QListWidget;
class QListWidgetItem;
class searchItem;
class Base;
class Singer;
class QMenu;
class QAction;
class QMutex;
class NetworkStatus;
#define ERROR(string)                                                          \
  qDebug() << __FILE__ << ' ' << __LINE__ << ' ' << string << '\n';
namespace Ui {
	class Search;
}

//保存解析json得到的数据
struct NetSongTag {
	int mvid;
	int song_id;
	int singer_id;
	int album_id;
	int status;
	QString duration;
	QString song_name;
	QString singer_name;
	QString album;
	NetSongTag()
		: song_id{}, singer_id{}, mvid{}, duration{}, song_name{}, album{},
		singer_name{}, album_id{} ,status{}{};
	~NetSongTag() {};
};

//歌曲是否可以播放
struct StateOfSong {
	bool success;
	QString message;
	StateOfSong() : success{ false }, message{} {};
};

//由于不止一次用于请求数据，所以用一个枚举类型来区分
enum class RequestType { SONGTYPE, STATETYPE, DETAILSTYPE, TOPSEARCHTYPE };
class Search : public QWidget {
	Q_OBJECT

public:
	explicit Search(QWidget* parent = nullptr);
	virtual ~Search();
	int curindex{};
	void InitTopSearchItem();
	void InitPlayListTabWiget();
	void InitMenu();
	void GetSearchText(QString str);
	void Parsejson(QJsonObject& root);
	void InitTableHeader();
	void ParseTopSearch(QJsonObject& root);
	void NetWorkState(QNetworkReply* reply);
	void TopSearch(QWidget* wgt);
	int CurInex() { return curindex; }
	void loadData();
	QStringList GetPlayList() {return songId;}
	QList<NetSongTag> getSearchResults() { return taglist; }
private slots:
	void on_btn_playall_clicked();
	void on_replyFinished(QNetworkReply*);
	void on_table_playlist_cellDoubleClicked(int row, int column);
	void on_btn_downloadall_clicked();

	//listwidg_song里的部件被点击
	void on_listwidg_song_itemClicked(QListWidgetItem*);
	void on_tabClicked(int index);
	void on_finshedSinger(QNetworkReply*);
signals:
	void play(Search*, const int index);
	void playAll(Search*);
	void Nextplay(Search*, const int index, const QString ID);
	void songID();
	void clickedTopSeach();
	void getid(const int ID);

public:
	/*热搜列表*/
	QListWidget* topsearchlist;
	//搜索top榜单的item
	QList<searchItem*> item;

private:
	QStringList songId{};
	NetworkStatus* Netstatus;
	//获取搜索栏字符串
	QString str{};
	QList<Singer> Singerlist{};
	QPixmap singerpic{};
	QMutex* mutex;
	//菜单
	QMenu* menu;
	QAction* Actplay;
	QAction* Actnextplay;
	QAction* Separator;
	QAction* Actdownload;
	QList<QAction*> Act{};
	bool cheacktop = false;
	Ui::Search* ui;
	Base* base;
	QString picUrl{};
	// M_Tag &tempTag = M_Tag::GetInstance();
	NetSongTag tag{};
	/*检查歌曲是否能播放*/
	StateOfSong state;
	/*临时tag*/
	QList<NetSongTag> taglist{};
	QList<Singer>SingerIdList{};
	QNetworkRequest* NetRequest;
	QNetworkAccessManager* NetManager;
	QNetworkAccessManager* NetSinger;
	QNetworkAccessManager* NetSingetPic;
	//区分每次请求的类型
	QMap<QNetworkReply*, RequestType> typeMap;
};

#endif // SEARCH_H
