#ifndef _M_TAG_H
#define _M_TAG_H
#include "config.h"
#include <QDialog>
#include <QList>
#include <QMap>
#include <QNetworkCookie>
#include <QPixmap>
#include <QString>
class QEventLoop;
class QNetworkReply;
class QJsonObject;
class Config;
class QNetworkCookie;
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class NetworkStatus : public QDialog {
	Q_OBJECT
public:
	NetworkStatus() {}
	virtual ~NetworkStatus() {}
	void NetStatus(QNetworkReply* reply);

signals:
	void timeout();
};

struct State {
	bool success;
	QString message;
	State() : success{ false }, message{} {}
};

//歌曲tag标签
struct Temptag {
	int Songid{};
	int AlbumId{};
	//喜欢音乐
	int status{};
	QString Artist{};
	QString Title{};
	QString Ablue{};
	QString Size{};
	QString Duration{};
	QPixmap AblueArt{};
};

//歌手
class Singer {
	int id{};
	int mvSize{};
	int musicSize{};
	int albumSize{};
	QString name{};
	QString picUrl{};
	QString alias{};
	QString briefDesc{};
	QPixmap pixmap{};


public:
	Singer() {}
	~Singer() {}
	void setid(const int _id) { id = _id; }
	void setMVSize(const int _mvs) { mvSize = _mvs; }
	void setmusicSize(const int _musicS) { musicSize = _musicS; }
	void setalbumSize(const int _albumS) { albumSize = _albumS; }
	void setname(const QString _name) { name = _name; }
	void setpicUrl(const QString _pciUrl) { picUrl = _pciUrl; }
	void setalias(const QString _alias) { alias = _alias; }
	void setbriefDesc(const QString _briefDesc) { briefDesc = _briefDesc; }
	void setpixmap(const QPixmap& _pixmap) { pixmap = _pixmap; }
	void clear() {
		id = 0;
		mvSize = 0;
		musicSize = 0;
		albumSize = 0;
		name.clear();
		picUrl.clear();
		alias.clear();
		picUrl.clear();
	};
	int getid() const { return id; }
	int getmvSize()const { return mvSize; }
	int getmusicSize() const { return musicSize; }
	int getalbumSize() const { return albumSize; }
	QPixmap& getPixmap() { return pixmap; }
	QString& getname() { return name; }
	QString& getpicUrl() { return picUrl; }
	QString& getbriefDesc() { return briefDesc; }
	QString& getalias() { return alias; }
};


class M_Tag : public QDialog {
	Q_OBJECT
public:
	M_Tag(QWidget* parent = nullptr);
	virtual ~M_Tag();
	QString GetArtist();
	QString GetTitle();
	QString GetAblue();
	QString GetSize();
	QString GetDuration();
	QPixmap GetAblueArt();

	// tag
	void SetArtist(const QString art) { tmptag.Artist = art; }
	void SetTitle(const QString titel) { tmptag.Title = titel; }
	void SetAblue(const QString ablue) { tmptag.Ablue = ablue; }
	void SetSize(const QString _size) { tmptag.Size = _size; }
	void SetStatus(const int _status) { tmptag.status = _status; }
	void SetDuration(const QString dur) { tmptag.Duration = dur; }
	void SetAblueArt(const QPixmap ico) { tmptag.AblueArt = ico; }
	//设置单曲ID
	void SetSongId(const int id) { tmptag.Songid = id; }
	void SetAlbumId(const int id) { tmptag.AlbumId = id; }
	void GetDetailsSong(const int id);
	void SetRequestHeader(QNetworkRequest*);
	bool ParseDetailsSong(QJsonObject& root, const QString& objname);
	//检查歌曲是否有版权
	void CheekState(const int index);
	State GetState() { return state; }
	QList<Temptag>* getTag() { return tag; };
	//网络超时
	void NetTimeOut(QNetworkReply* reply);
	//喜欢音乐
	void likeMusic(const int, const bool);
private slots:
	void on_replyFinshed(QNetworkReply* reply);
	void on_GetAblueArt(QNetworkReply* reply);
signals:
	void parseOk();


private:
	Temptag tmptag{};
	State state{};
	QList<Temptag>* tag;
	Config conf{};
	QEventLoop* eventloop;
	QNetworkRequest* Request;
	QNetworkAccessManager* NetManager;
	QNetworkAccessManager* NetPase;
	//检查歌曲是否有版权
	QNetworkAccessManager* NetCheek;

	QNetworkAccessManager* Netlike;
	QList<QNetworkCookie> cookies;
};
// static M_Tag gettag(M_Tag *);
#endif // _M_TAG_H
