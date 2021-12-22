#include "search.h"
#include "base.h"
#include "networkutil.h"
#include "ui_search.h"
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QString>
// http://cloud-music.pl-fe.cn/
Search::Search(QWidget *parent) : QWidget(parent), ui(new Ui::Search) {
  ui->setupUi(this);
  base = new Base(ui->table_playlist);
  InitTableHeader();
  this->setWindowFlags(Qt::X11BypassWindowManagerHint |
                       Qt::FramelessWindowHint);

  InitPlayListTabWiget();
  //初始化表头

  ui->scrollArea->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);
  ui->tabWidget->setWindowFlags(Qt::CustomizeWindowHint |
                                Qt::FramelessWindowHint);
  ui->table_playlist->setWindowFlags(Qt::CustomizeWindowHint |
                                     Qt::FramelessWindowHint);

  NetManager = new QNetworkAccessManager(this);
  connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this,
          &Search::on_replyFinished);
}

Search::~Search() { delete ui; }

void Search::InitPlayListTabWiget() {
  ui->tabWidget->setStyleSheet("QTabBar::tab{width:120px;height:30px;");
}

void Search::GetSearchText(QString &str) {
  ui->lab_song->setText(str);
  QString url =
      QString("http://cloud-music.pl-fe.cn/search?keywords=%1?type=100")
          .arg(str);
  QNetworkReply *Reply = NetWorkUtil::instance()->get(url);
  typeMap.insert(Reply, RequestType::songType);
}

void Search::Parsejson(QJsonObject &root) {
  //解析请求到的json文件
  SearchResults.clear();
  // QJsonParseError eeor_t{};
  // QJsonDocument deocument = QJsonDocument::fromJson(byte, &eeor_t);
  // if (eeor_t.error == QJsonParseError::NoError) {
  //  QJsonObject root = deocument.object();
  QJsonValue result = root.value("result");
  if (result.isObject()) {
    QJsonObject resultobj = result.toObject();
    auto songs = resultobj.value("songs");
    if (songs.isArray()) {
      QJsonArray songs_array = songs.toArray();
      for (auto it = songs_array.begin(); it != songs_array.end(); ++it) {
        QJsonValue songs = songs_array.at(it.i);
        //获得第一层的json对象的value
        if (songs.isObject()) {
          QJsonObject songsobj = songs.toObject();
          s_results.song_id = songsobj.value("id").toInt();
          s_results.song_name = songsobj.value("name").toString();
          s_results.duration = songsobj.value("duration").toInt();
          s_results.mvid = songsobj.value("mvid").toInt();
          auto artists = songsobj.value("artists");
          if (artists.isArray()) {
            auto artits_ary = artists.toArray();
            auto artits = artits_ary.at(0);
            if (artits.isObject()) {
              auto artitsobj = artits.toObject();
              s_results.singer_id = artitsobj.value("id").toInt();
              s_results.singer_name = artitsobj.value("name").toString();
            }
          }
          //解析专辑这个json对象
          auto album = songsobj.value("album");
          if (album.isObject()) {
            auto albumobj = album.toObject();
            s_results.album_id = albumobj.value("id").toInt();
            s_results.album = albumobj.value("name").toString();
            //将其加入list<SearchResults>
            SearchResults.push_back(s_results);
          }
        }
      }
    }
  }
  base->DelTableWidgetRow();
  for (int i = 0; i != SearchResults.count(); ++i) {
    QString song_name = SearchResults.at(i).song_name;
    QString singer_name = SearchResults.at(i).singer_name;
    QString album = SearchResults.at(i).album;
    //将时间转换为分钟和秒钟
    QString duration =
        QString("%1 : %2")
            .arg(SearchResults.at(i).duration / 1000 / 60, 2, 10, QChar('0'))
            .arg((int)SearchResults.at(i).duration % 60, 2, 10, QChar('0'));
    QStringList _list{song_name, singer_name, album, duration};
    base->InsertDataInfoTableWidget(_list, i);
  }
}
//}

void Search::InitTableHeader() {
  QStringList HorizontalHeaderItem{
      QObject::tr("音乐标题"),
      QObject::tr("歌手"),
      QObject::tr("专辑"),
      QObject::tr("时长"),
  };
  --base->column;
  //获取列数
  int column = HorizontalHeaderItem.count();
  ui->table_playlist->setColumnCount(column);
  //设置行高
  ui->table_playlist->setLineWidth(10);
  for (int x = 0; x != column; ++x) {
    ui->table_playlist->setHorizontalHeaderItem(
        x, new QTableWidgetItem(HorizontalHeaderItem.at(x)));
  }
}

void Search::ParseSongState(QJsonObject &root) {
  QJsonValue _obj = root;
  if (_obj.isObject()) {
    state.success = _obj.toObject().value("success").toBool();
    state.message = _obj.toObject().value("message").toString();
  }
}

void Search::NetWorkState(QNetworkReply *reply) {
  switch (reply->error()) {
  case QNetworkReply::TimeoutError:
    QMessageBox::critical(this, tr("tip"),
                          tr("网络链接超时，请检查网络设置情况\n"),
                          QMessageBox::YesAll);
    ERROR("QNetworkReply TimeoutError");
    break;
  case QNetworkReply::NetworkSessionFailedError:
    QMessageBox::critical(this, tr("tip"),
                          tr("由于网络断开或启动网络失败，导致连接中断\n"),
                          QMessageBox::YesAll);
    ERROR("NetworkSessionFailedError");
    break;
  case QNetworkReply::ProxyTimeoutError:
    QMessageBox::critical(this, tr("tip"),
                          tr("与代理的连接超时或代理没有及时回复发送的请求\n"),
                          QMessageBox::YesAll);
    ERROR("ProxyTimeoutError");
    break;
  default:
    break;
  }
}

void Search::ParseSongDetails(QJsonObject &root) {
  QJsonValue value = root.value("songs");
  if (value.isArray()) {
    auto songary = value.toArray();
    if (songary.at(0).isObject()) {
      auto songobj = songary.at(0).toObject();
      auto alVule = songobj.value("al");
      if (alVule.isObject()) {
        auto alobj = alVule.toObject();
        picUrl = alobj.value("picUrl").toString();
        //获取歌曲专辑封面
        NetManager->get(QNetworkRequest(picUrl));
        //获取专辑封面
        // connect(NetManager, &QNetworkAccessManager::finished, this,
        //        [&](QNetworkReply *reply) { //获取专辑封面
        //          if (reply->error() == QNetworkReply::NoError) {
        //            AlbumArt.loadFromData(reply->readAll());
        //          }
        //        });
        return;
      }
    }
  }
}




void Search::on_replyFinished(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << reply->errorString();
    NetWorkState(reply);
    return;
  }

  QByteArray byte{reply->readAll()};
  QJsonParseError eeor_t{};
  QJsonObject root{};
  QJsonDocument deocument = QJsonDocument::fromJson(byte, &eeor_t);
  if (eeor_t.error != QJsonParseError::NoError) {
    QMessageBox::warning(this, tr("warning"), tr("Jsong format error\n"),
                         QMessageBox::YesAll);
    ERROR("Jsong format error");
    return;
  } else {
    root = deocument.object();
  }

  RequestType reType = typeMap.value(reply);
  switch (reType) {
  case RequestType::songType:
    Parsejson(root);
    ui->btn_details->setText(
        QString("歌手：%1").arg(SearchResults.at(0).singer_name));
    break;
  case RequestType::stateType:
    ParseSongState(root);
    if (state.success == true) {
      qDebug() << "song success\n";

    } else {
      qDebug() << state.message << '\n';
    }
  case RequestType::Song_Details:
    //获取mp3的封面url地址
    ParseSongDetails(root);
    break;
  default:
    break;
  }
  // reply需要我们自行delete掉，需使用deleteLater进行释放
  reply->deleteLater();
}

void Search::on_btn_playall_clicked() {
  playlistID.clear();
  for (int i = 0; i != SearchResults.count(); ++i) {
    playlistID.push_back(QString("%1").arg(SearchResults.at(i).song_id));
  }
  QString MusicID = playlistID.at(0);
  qDebug() << "ID =" << MusicID << '\n';
 
  curindex = 0;
  
  emit play(playlistID.at(0));
  //
}

void Search::getAlbumPic(const int i) { //获取歌曲详情，取得imageurl地址
  //QNetworkReply *reply = NetWorkUtil::instance()->get(
      //QString("http://cloud-music.pl-fe.cn/song/detail?ids=%1").arg(SearchResults.ser));

  //typeMap.insert(reply, RequestType::Song_Details);
}

void Search::ParseAlbumPic(const int n) {

}


void Search::on_table_playlist_cellDoubleClicked(int row, int column) {
  Q_UNUSED(column);
  int id = SearchResults.at(row).song_id;
  QNetworkReply *Reply = NetWorkUtil::instance()->get(
      QString("http://cloud-music.pl-fe.cn/check/music?id=%1").arg(id));
  typeMap.insert(Reply, RequestType::stateType);
}

//下载全部
void Search::on_btn_downloadall_clicked() {}

// size == 0
void Search::on_parsepci(QNetworkReply *reply) {
  qDebug() << " reply->size(): " << reply->size() << "\n";
  if (reply->error() == QNetworkReply::NoError) {
    AlbumArt.loadFromData(reply->readAll());

    return;
  }
}
