#include "tag.h"
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
M_Tag::M_Tag(QWidget* parent) : QDialog(parent) {
	NetPase = new QNetworkAccessManager(this);
	NetManager = new QNetworkAccessManager(this);
	NetCheek = new QNetworkAccessManager(this);
	Netlike = new QNetworkAccessManager(this);
	Request = new QNetworkRequest();
	eventloop = new QEventLoop(this);
	tag = new QList<Temptag>;
	SetRequestHeader(Request);

	connect(NetPase, &QNetworkAccessManager::finished, this,
		&M_Tag::on_replyFinshed);

	//获取专辑封面
	connect(NetManager, &QNetworkAccessManager::finished, this,
		&M_Tag::on_GetAblueArt);

	connect(NetCheek, &QNetworkAccessManager::finished, this,
		[&](QNetworkReply* reply) {
			if (reply->error() == QNetworkReply::NoError) {
				QByteArray byt = reply->readAll();
				QJsonParseError error_t{};
				QJsonObject root{};
				QJsonDocument deocument = QJsonDocument::fromJson(byt, &error_t);
				root = deocument.object();
				if (error_t.error == QJsonParseError::NoError) {
					QJsonValue _obj = root;
					state.success = _obj.toObject().value("success").toBool();
					state.message = _obj.toObject().value("message").toString();
				}
			}
			reply->deleteLater();
		});

	//
	connect(Netlike, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
		if (reply->error() == QNetworkReply::NoError) {

		}
		reply->deleteLater();
		});
}

M_Tag::~M_Tag() { 
	qDebug() << "~M_Tag()";
	delete tag;
	tag = nullptr;
	eventloop->exit(); 
}

QString M_Tag::GetArtist() { return tmptag.Artist; }

QString M_Tag::GetTitle() { return tmptag.Title; }

QString M_Tag::GetAblue() { return tmptag.Ablue; }

QString M_Tag::GetSize() { return tmptag.Size; }

QString M_Tag::GetDuration() { return tmptag.Duration; }

QPixmap M_Tag::GetAblueArt() { return tmptag.AblueArt; }

void M_Tag::SetRequestHeader(QNetworkRequest* req) {
	if (conf.GetCookies().isEmpty()) {
		qDebug() << "未找到cookie,某些功能无法使用，请登录账号！\n";
		return;
	}
	//解析cookies
	cookies = QNetworkCookie::parseCookies(conf.GetCookies());
	QNetworkCookieJar* jar = new QNetworkCookieJar(this);
	//向请求头里加入cookies
	QVariant var{};
	var.setValue(cookies);
	//设置请求头
	req->setHeader(QNetworkRequest::CookieHeader, var);
}

void M_Tag::GetDetailsSong(const int id) {
	// if (id.isEmpty())
	// return;
	//歌曲详情，(首先需要拿到歌曲ID)
	CheekState(id);
	Request->setUrl(
		QString("http://localhost:3000/song/detail?ids=%1").arg(id));
	NetPase->get(*Request);

}

//http://localhost:3000/like?id=411214279&like=false
//like = false 取消喜欢
void M_Tag::likeMusic(const int ID, const bool like)
{
	QNetworkRequest* request{ conf.setCookies() };
	request->setUrl(QString("http://localhost:3000/like?id=%1&like=%2").arg(ID).arg(like));
	Netlike->get(*request);
}

void M_Tag::on_replyFinshed(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray byte = reply->readAll();
		QJsonParseError error_t{};
		QJsonDocument deocument = QJsonDocument::fromJson(byte, &error_t);
		if (error_t.error == QJsonParseError::NoError) {
			QJsonObject root = deocument.object();
			//开始解析Json 获取歌曲详情
			ParseDetailsSong(root, "songs");
		}
		else {
			fprintf(stdout, "QJson format error!\n");
		}
	}
	reply->deleteLater();
}

bool M_Tag::ParseDetailsSong(QJsonObject& root, const QString& objname) {
	tag->clear();
	QJsonValue value = root.value(objname);
	if (value.isArray()) {
		auto songary = value.toArray();
		foreach(const QJsonValue rhs, songary) {
			if (rhs.isObject()) {
				auto songobj = rhs.toObject();
				auto alVule = songobj.value("al");
				if (alVule.isObject()) {
					auto alobj = alVule.toObject();
					SetAlbumId(alobj.value("id").toInt());
					SetAblue(alobj.value("name").toString());
					QString pciUrl = alobj.value("picUrl").toString();
					//获取专辑封面
					NetManager->get(QNetworkRequest(pciUrl));
				}

				SetTitle(songobj.value("name").toString());
				SetSongId(songobj.value("id").toInt());
				//歌曲时长
				int dt = songobj.value("dt").toInt();
				QString t = QString("%1 : %2")
					.arg(dt / 1000 / 60, 2, 10, QChar('0'))
					.arg(dt % 60, 2, 10, QChar('0'));
				SetDuration(t);

				auto ar = songobj.value("ar");
				if (ar.isArray()) {
					auto arary = ar.toArray();
					SetArtist(arary.at(0).toObject().value("name").toString());
				}
				SetStatus(songobj.value("status").toInt());
			}
			tag->push_back(tmptag);
		}
		emit parseOk();
		return true;
	}
	return false;
}

void M_Tag::CheekState(const int index) {
	Request->setUrl(
		QString("http://localhost:3000/check/music?id=%1").arg(index));
	SetRequestHeader(Request);
	NetCheek->get(*Request);
}

void M_Tag::on_GetAblueArt(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QPixmap map{};
		map.loadFromData(reply->readAll());
		SetAblueArt(map);
		//发送信号设置底栏按钮，不发信息一直出问题(每次设置tag信息都对不上)
		emit parseOk();
	}
	reply->deleteLater();
}

//网络请求超时
void NetworkStatus::NetStatus(QNetworkReply* reply) {
	QTimer time;
	// 30s
	time.setSingleShot(30000);
	//单词触发
	time.setSingleShot(true);

	QEventLoop loop;
	connect(&time, &QTimer::timeout, &loop, &QEventLoop::quit);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	//开始计时
	time.start();
	//启动事件循环
	loop.exec();
	//处理相应
	if (time.isActive()) {
		time.stop();
		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << "Error String:" << reply->errorString() << endl;
		}
		else {
			//获取状态码
			QVariant variant =
				reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
			int nStatusCode = variant.toInt();
			qDebug() << "Status Code: " << nStatusCode << endl;
		}
	}
	else {
		//超时处理
		disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		reply->abort();
		reply->deleteLater();
		emit timeout();
		QLabel* lbe = new QLabel("Network timeout");
	}
}
