#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "recommend.h"
#include "soloalbum.h"
#include "tag.h"
#include "ui_recommend.h"
#include <QDateTime>
#include <QEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMouseEvent>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTabWidget>
#include <QTimer>
#include <thread>
Recommend::Recommend(QWidget *parent) : QWidget(parent), ui(new Ui::Recommend) {
  ui->setupUi(this);
  InitLabel();
  mutex = new QMutex();
  ui->lab_1->installEventFilter(this);
  ui->lab_2->installEventFilter(this);
  ui->lab_3->installEventFilter(this);
  // ui->lab_1->setMouseTracking(true);
  // ui->lab_2->setMouseTracking(true);
  // ui->lab_3->setMouseTracking(true);

  time = new QTimer(this);
  m_tag = new M_Tag(this);
  soloalbum = new SoloAlbum(this);
  soloalbum->hide();
  NetMangBanner = new QNetworkAccessManager(this);
  NetGetBanner = new QNetworkAccessManager(this);
  NetNewDisc = new QNetworkAccessManager(this);
  NetNewSong = new QNetworkAccessManager(this);
  NetAlbumPic = new QNetworkAccessManager(this);

  QString url{"http://cloud-music.pl-fe.cn/banner?type=0"};

  request = new QNetworkRequest(url);
  NetMangBanner->get(*request);
  connect(NetMangBanner, &QNetworkAccessManager::finished, this,
          &Recommend::on_BannerReplyFinished);

  connect(NetGetBanner, &QNetworkAccessManager::finished, this,
          &Recommend::on_GetBannerPic);

  connect(NetNewDisc, &QNetworkAccessManager::finished, this,
          &Recommend::on_FinshedNewDisc);

  connect(NetAlbumPic, &QNetworkAccessManager::finished, this,
          &Recommend::on_FinshedGetAlubPic);
  //每5秒更换一个照片墙
  time->setInterval(5000);
  time->start();
  //connect(time, &QTimer::timeout, this, &Recommend::NextPic);

  connect(ui->btn_prev, &QPushButton::clicked, this, [&] {
    --index;
    Prevpic();
    time->setInterval(5000);
  });
  connect(ui->btn_next, &QPushButton::clicked, this, [&] {
    NextPic();
    time->setInterval(5000);
  });
}

Recommend::~Recommend() {
  delete ui;
  delete request;
  request = nullptr;
  delete mutex;
  mutex = nullptr;
}

//加载本地图片
void Recommend::LoadPic() {
  QPixmap pix{};
  for (int x = 0; x != 10; ++x) {
    pix.load(QString("../photowall/%1").arg(x));
    pixmap.push_back(pix);
  }
  setPic(0);
}

void Recommend::setPic(const int _index) {
  ui->lab_2->setPixmap(pixmap.at(9));
  ui->lab_1->setPixmap(pixmap.at(_index));
  ui->lab_3->setPixmap(pixmap.at(1));
  ui->lab_title_1->setText(QString("%1").arg(targetlist.at(index).typeTitle));
  ui->lab_title_2->setText(
      QString("%1").arg(targetlist.at(index + 1).typeTitle));
}

void Recommend::NextPic() {
  fprintf(stdout, "index = %d\n", index);
  if (index == 9) {
    fprintf(stdout, "index == %d\n", index);
    ui->lab_2->setPixmap(pixmap.at(index - 1));
    ui->lab_1->setPixmap(pixmap.at(index));
    ui->lab_3->setPixmap(pixmap.at(0));
    index = 0;
    return;
  }
  //回到开始
  if (index == 0) {
    setPic(0);
    ++index;
    return;
  }
  ui->lab_2->setPixmap(pixmap.at(index - 1));
  ui->lab_1->setPixmap(pixmap.at(index));
  ui->lab_3->setPixmap(pixmap.at(index + 1));
  ui->lab_title_1->setText(QString("%1").arg(targetlist.at(index).typeTitle));
  ui->lab_title_2->setText(
      QString("%1").arg(targetlist.at(index + 1).typeTitle));
  ++index;
  // fprintf(stdout, "index = %d\n", index);
}

void Recommend::Prevpic() { fprintf(stdout, "prev index = %d\n", index); }

void Recommend::InitLabel() {
  lablist.push_back(ui->lab_1);
  lablist.push_back(ui->lab_2);
  lablist.push_back(ui->lab_3);
  //缩放位图填空间
  for (const auto &x : lablist) {
    x->setScaledContents(true);
  }
}

bool Recommend::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->lab_1) {
    //鼠标进入事件
    if (event->type() == QEvent::Enter) {
    } else if (event->type() == QEvent::Leave) {
    }

    // label被点击
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouse = static_cast<QMouseEvent *>(event);

      if (mouse->button() == Qt::LeftButton) {
        if (!strcmp(targetlist.at(index - 1).typeTitle.toUtf8().data(),
                    "新碟首发")) {
          fprintf(stdout, "enter Disc\n");
          NetNewDisc->get(
              QNetworkRequest(QString("http://cloud-music.pl-fe.cn/album?id=%1")
                                  .arg(targetlist.at(index - 1).targetId)));
        } else {
          NetNewSong->get(QNetworkRequest(
              QString("http://cloud-music.pl-fe.cn/song/detail?ids=%1")
                  .arg(targetlist.at(index).targetId)));
        }
      }
    }
  }
  return QWidget::eventFilter(obj, event);
}

void Recommend::getPic() {
  //加锁，防止没保存完图片就去读取，照成的数据越界
  mutex->lock();
  // QEventLoop evenloop;
  // QNetworkReply *reply = nullptr;
  foreach (auto &val, targetlist) {
    // reply =
    // NetGetBanner->get(QNetworkRequest(QString("%1").arg(val.picUrl)));
    NetGetBanner->get(QNetworkRequest(QString("%1").arg(val.picUrl)));
    // connect(reply, &QNetworkReply::finished, &evenloop, &QEventLoop::quit);
    // evenloop.exec(QEventLoop::ExcludeUserInputEvents);
  }
  mutex->unlock();
  //加载图片到照片墙
  this->LoadPic();
}

void Recommend::on_BannerReplyFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byt = reply->readAll();
    QJsonParseError error_t{};
    QJsonDocument docm = QJsonDocument::fromJson(byt, &error_t);
    if (error_t.error == QJsonParseError::NoError) {
      QJsonObject root = docm.object();
      auto Banrot = root.value("banners");
      if (Banrot.isArray()) {
        QJsonArray BanArry = Banrot.toArray();
        foreach (const auto &x, BanArry) {
          if (x.isObject()) {
            auto Ban_r = x.toObject();
            target.targetId = Ban_r.value("targetId").toInt();
            target.typeTitle = Ban_r.value("typeTitle").toString();
            target.picUrl = Ban_r.value("imageUrl").toString();
            //取得轮播图的链接
            targetlist.push_back(target);
          }
        }
      }
    }
    //请求轮播图,不阻塞当前线程
    this->getPic();
    // std::thread t([&] { getPic(); });
    // t.detach();
  }
  reply->deleteLater();
}
void Recommend::on_GetBannerPic(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QPixmap map;
    map.loadFromData(reply->readAll());
    static int n = 0;
    map.save(QString("../photowall/%1.png").arg(n));
  }
  reply->deleteLater();
}

//返回专辑Ui界面
SoloAlbum *Recommend::getAlbumUi() { return soloalbum; }

//发布的新歌
void Recommend::on_FinshedNewSong(QNetworkReply *reply) {}

void Recommend::on_FinshedGetAlubPic(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QPixmap pixmap;
    pixmap.loadFromData(reply->readAll());
    //设置专辑封面
    soloalbum->setlab_AlubPic(pixmap);
  }
  reply->deleteLater();
}

//专辑
void Recommend::on_FinshedNewDisc(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byt = reply->readAll();
    QJsonParseError error_t{};
    Albumtag tag;
    QJsonDocument docm = QJsonDocument::fromJson(byt, &error_t);
    if (error_t.error == QJsonParseError::NoError) {
      QJsonObject root = docm.object();
      QJsonValue songs = root.value("songs");
      if (songs.isArray()) {
        QJsonArray songAry = songs.toArray();
        foreach (const auto &i, songAry) {
          if (i.isObject()) {
            auto songRot = i.toObject();
            //解析歌手和id
            QJsonValue ar = songRot.value("ar");
            if (ar.isArray()) {
              QJsonArray atAry = ar.toArray();
              QJsonObject arrot = atAry.at(0).toObject();
              tag.Artist_id = arrot.value("id").toInt();
              tag.Artist = arrot.value("name").toString();
            }

            //专辑名称
            QJsonValue al = songRot.value("al");
            if (al.isObject()) {
              QJsonObject alobj = al.toObject();
              tag.Album_id = alobj.value("id").toInt();
              tag.Album = alobj.value("name").toString();
            }
            //解析歌曲title
            tag.Title = songRot.value("name").toString();
            tag.Song_id = songRot.value("id").toInt();

            //歌曲时长
            int dt = songRot.value("dt").toInt();
            tag.Duration = QString("%1 : %2")
                               .arg(dt / 1000 / 60, 2, 10, QChar('0'))
                               .arg(dt % 60, 2, 10, QChar('0'));
          }

          //解析的数据储存起来，方便以后使用
          soloalbum->setAlbumtag(tag);
        }
      }
      //专辑描述
      QJsonValue album = root.value("album");
      if (album.isObject()) {
        QJsonObject albobj = album.toObject();
        //获取专辑描述
        soloalbum->setDescription(albobj.value("description").toString());
        //获取发布时间
        long long pt = albobj.value("publishTime").toVariant().toLongLong();
        QDateTime dd = QDateTime::fromMSecsSinceEpoch(pt);
        QString s = dd.toString("yyyy-MM-dd");
        soloalbum->setPublishTime(dd.toString("yyyy-MM-dd"));
        //设置歌手
        soloalbum->setlab_title(albobj.value("name").toString());
        QString url = albobj.value("picUrl").toString();
        NetAlbumPic->get(QNetworkRequest(url));

        //获取该专辑的歌手
        QJsonValue art = albobj.value("artists");
        if (art.isArray()) {
          QJsonArray artAry = art.toArray();
          foreach (const QJsonValue &rhs, artAry) {
            if (rhs.isObject()) {
              QJsonObject artobj = rhs.toObject();
              QString artist = artobj.value("name").toString();
              soloalbum->setlab_Artist(artist);
            }
          }
        }
      }
      soloalbum->LoadData();
    }
  }
  reply->deleteLater();
}