#include "tag.h"
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>

M_Tag::M_Tag(QWidget *parent) : QDialog(parent) {
  NetPase = new QNetworkAccessManager(this);
  NetManager = new QNetworkAccessManager(this);
  NetCheek = new QNetworkAccessManager(this);
  Request = new QNetworkRequest();
  eventloop = new QEventLoop(this);
  SetRequestHeader(Request);

  connect(NetPase, &QNetworkAccessManager::finished, this,
          &M_Tag::on_replyFinshed);

  //获取专辑封面
  connect(NetManager, &QNetworkAccessManager::finished, this,
          &M_Tag::on_GetAblueArt);

  connect(NetCheek, &QNetworkAccessManager::finished, this,
          [&](QNetworkReply *reply) {
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
                if (state.success == false) {
                int clicked =  QMessageBox::warning(this, "Warning", state.message);
                  if (clicked == QMessageBox::Yes) {
                  emit NoCopyright();
                  }
                }
              }
            }
            reply->deleteLater();
          });
}

M_Tag::~M_Tag() { eventloop->exit(); }

QString M_Tag::GetArtist() { return Artist; }

QString M_Tag::GetTitle() { return Title; }

QString M_Tag::GetAblue() { return Ablue; }

QString M_Tag::GetSize() { return Size; }

QString M_Tag::GetDuration() { return Duration; }

QPixmap M_Tag::GetAblueArt() { return AblueArt; }

void M_Tag::insertUrl(const int index, const int _id) { id.insert(index, _id); }

void M_Tag::SetRequestHeader(QNetworkRequest *req) {
  if (conf.GetCookies().isEmpty()) {
    qDebug() << "未找到cookie,某些功能无法使用，请登录账号！\n";
    return;
  }
  //解析cookies
  cookies = QNetworkCookie::parseCookies(conf.GetCookies());
  QNetworkCookieJar *jar = new QNetworkCookieJar(this);
  //向请求头里加入cookies
  QVariant var{};
  var.setValue(cookies);
  //设置请求头
  req->setHeader(QNetworkRequest::CookieHeader, var);
}

void M_Tag::SetSongId(const QList<int> &id_list) {
  id.clear();
  id = id_list;
}

void M_Tag::GetDetailsSong(const int index) {
  if (id.isEmpty())
    return;
  //歌曲详情
  Request->setUrl(QString("http://cloud-music.pl-fe.cn/song/detail?ids=%1")
                      .arg(id.at(index)));
  NetPase->get(*Request);
}

void M_Tag::on_replyFinshed(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    static int n = 0;
    qDebug() << "come  " << __FILE__ << " " << __LINE__ << " " << n << "\n";
    ++n;
    QByteArray byte = reply->readAll();
    QJsonParseError error_t{};
    QJsonObject root{};
    QJsonDocument deocument = QJsonDocument::fromJson(byte, &error_t);
    root = deocument.object();
    //开始解析Json 获取歌曲详情
    ParseDetailsSong(root);
  }
  reply->deleteLater();
}

void M_Tag::ParseDetailsSong(QJsonObject &root) {
  QJsonValue value = root.value("songs");
  if (value.isArray()) {
    auto songary = value.toArray();
    QJsonValue s = songary.at(0);
    if (s.isObject()) {
      auto songobj = s.toObject();
      SetTitle(songobj.value("name").toString());
      auto ar = songobj.value("ar");
      if (ar.isArray()) {
        auto arary = ar.toArray();
        SetArtist(arary.at(0).toObject().value("name").toString());
      }
      auto alVule = songobj.value("al");
      if (alVule.isObject()) {
        auto alobj = alVule.toObject();
        SetAblue(alobj.value("name").toString());
        QString pciUrl = alobj.value("picUrl").toString().toLocal8Bit();
        qDebug() << __FILE__ << " pciUrl: " << pciUrl << "\n";
        //获取专辑封面
        NetManager->get(QNetworkRequest(pciUrl));
      }
    }
  }
}

void M_Tag::CheekState(const int index) {
  Request->setUrl(QString("http://cloud-music.pl-fe.cn/check/music?id=%1")
                      .arg(id.at(index)));
  SetRequestHeader(Request);
  NetCheek->get(*Request);
}

void M_Tag::on_GetAblueArt(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byte = reply->readAll();
    QPixmap map;
    map.loadFromData(byte);
    SetAblueArt(map);
    emit
    parseOk(); //发送信号设置底栏按钮，不发信息一直出问题(每次设置tag信息都对不上)
  }
  reply->deleteLater();
}