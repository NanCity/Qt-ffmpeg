#include "cloudmusic.h"
#include "base.h"
#include "config.h"
#include "tag.h"
#include "ui_cloudmusic.h"
#include <QAction>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
CloudMusic::CloudMusic(QWidget *parent)
    : QWidget(parent), ui(new Ui::CloudMusic) {
  ui->setupUi(this);
  config = new Config();

  base = new Base(ui->tableWidget);
  Delmang = new QNetworkAccessManager(this);
  InitHorizontalHead();
  InitMenu();
  //允许QTableWidget接收QWidget::customContextMenuRequested()信号。
  ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  Manger = new QNetworkAccessManager(this);

  connect(Manger, &QNetworkAccessManager::finished, this,
          &CloudMusic::on_replyFinished);

  connect(Delmang, &QNetworkAccessManager::finished, this,
          [](QNetworkReply *reply) {
            if (reply->error() == QNetworkReply::NoError) {
              qDebug() << "Delete ok\n"
                       << reply->readAll().toStdString().c_str();
            }
          });
}

CloudMusic::~CloudMusic() {
  delete ui;
  delete config;
  config = nullptr;
}

void CloudMusic::LoadMyCloud() { RequestResources(Manger); }

void CloudMusic::InitHorizontalHead() {
  QStringList list{"Title",  "Artist", "Album",
                   "Format", "Size",   "Upload Time "};
  int column = list.length();
  ui->tableWidget->setColumnCount(column);
  ui->tableWidget->setLineWidth(10);
  for (int x = 0; x != column; ++x) {
    ui->tableWidget->setHorizontalHeaderItem(x,
                                             new QTableWidgetItem(list.at(x)));
  }
}

void CloudMusic::InitMenu() {
  menu = new QMenu(ui->tableWidget);
  ActPlay =
      new QAction(QIcon(":/images/bottom/btn_play_h.png"), "播放(play)", menu);
  ActNextPlay = new QAction(QIcon(":/images/bottom/btn_single_h.png"),
                            "下一首播放(next play)", menu);
  ActDownload = new QAction(QIcon(""), "下载(Download)", menu);
  ActDelete =
      new QAction(QIcon(":/images/btn_delete_h.png"), "删除(Delete)", menu);
  //加入菜单栏
  menu->addAction(ActPlay);
  menu->addAction(ActNextPlay);
  menu->addSeparator();
  menu->addAction(ActDownload);
  menu->addSeparator();
  menu->addAction(ActDelete);
  connect(ActPlay, &QAction::triggered, this, [&]() {
    int id = clist.at(ui->tableWidget->currentRow()).songid;
    emit play(this, id);
  });
  connect(ActNextPlay, &QAction::triggered, this, [&]() {});
  connect(ActDownload, &QAction::triggered, this, [&]() {});

  connect(ActDelete, &QAction::triggered, this, &CloudMusic::DeleteSong);
}

void CloudMusic::RequestResources(QNetworkAccessManager *manger) {
  //请求地址
  QString url = "http://cloud-music.pl-fe.cn/user/cloud?limit=100";
  request = config->setCookies();
  request->setUrl(url);
  manger->get(*request);
  //云盘，需要cookie才能正确获取
}

void CloudMusic::on_replyFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byte = reply->readAll();
    QJsonParseError err_rpt{};
    // QJsonDocument封装了完整的json文档
    QJsonDocument document = QJsonDocument::fromJson(byte, &err_rpt);
    if (err_rpt.error == QJsonParseError::NoError) {
      QJsonObject rootobj = document.object();
      ParseJson(rootobj);
    }
  }
  reply->deleteLater();
}

void CloudMusic::ParseJson(QJsonObject &rootobj) {
  base->DelTableWidgetRow();
  auto data = rootobj.value("data");
  QStringList taglist;
  int n = 0;
  if (data.isArray()) {
    QJsonArray dataArray = data.toArray();
    for (auto it = dataArray.begin(); it != dataArray.end(); ++it) {
      auto obj = dataArray.at(it.i);

      if (obj.isObject()) {
        QJsonObject simpleObj = obj.toObject();
        auto simple = simpleObj.value("simpleSong");
        // simpleSong对象
        if (simple.isObject()) {
          auto _simpobj = simple.toObject();
          //歌手
          if (_simpobj.value("ar").isArray()) {
            auto arAry = _simpobj.value("ar").toArray();
            if (arAry.at(0).isObject()) {
              auto idobj = arAry.at(0).toObject();
              cloundtag.artid = idobj.value("id").toInt();
              cloundtag.artist = idobj.value("name").toString();
              if (cloundtag.artist.isEmpty()) {
                cloundtag.artist = "未知歌手";
              }
            }

            //专辑
            if (_simpobj.value("al").isObject()) {
              auto alAy = _simpobj.value("al").toObject();
              cloundtag.albumid = alAy.value("id").toInt();
              cloundtag.album = alAy.value("name").toString();
              cloundtag.lyricId = alAy.value("id").toInt();
              if (cloundtag.album.isEmpty()) {
                cloundtag.album = "未知专辑";
              }
              cloundtag.picUrl = alAy.value("picUrl").toString();
            }
          }
        }
      }

      //提取歌曲格式
      QString format =
          obj.toObject().value("fileName").toString().section('.', -1);
      cloundtag.format = format;
      int fszie = obj.toObject().value("fileSize").toInt();
      cloundtag.fileSize =
          QString("%1").number((double)fszie / 1024 / 1024, 'f', 2) + " MB";
      cloundtag.songid = obj.toObject().value("songId").toInt();
      long long addt = obj.toObject().value("addTime").toVariant().toLongLong();
      ////时间戳-毫秒级
      QDateTime dd = QDateTime::fromMSecsSinceEpoch(addt);
      cloundtag.addTime = dd.toString("yyyy-MM-dd");
      cloundtag.songName = obj.toObject().value("songName").toString();
      //添加入list
      clist.push_back(cloundtag);
      QStringList list{cloundtag.songName, cloundtag.artist,
                       cloundtag.album,    cloundtag.format,
                       cloundtag.fileSize, cloundtag.addTime};
      base->InsertDataInfoTableWidget(list, n);
      ++n;
    }
  } else {
    QMessageBox::warning(this, "Warning", "未登录账号,无法查询网盘数据\n",
                         QMessageBox::Yes);
    return;
  }
}

void CloudMusic::DeleteSong() {
  int currentrow = ui->tableWidget->currentRow();
  //获取云盘音乐的ID，之后进行删除
  int deId = clist.at(currentrow).songid;
  request->setUrl(
      QString("http://cloud-music.pl-fe.cn/user/cloud/del?id=%1").arg(deId));
  RequestResources(Delmang);
  ui->tableWidget->removeRow(currentrow);
}

void CloudMusic::on_tableWidget_customContextMenuRequested(const QPoint &pos) {
  if (ui->tableWidget->itemAt(pos) != nullptr) {
    menu->exec(QCursor::pos());
  }
}

void CloudMusic::on_lineEdit_textChanged(const QString &text) {
  base->SerachData(text);
}