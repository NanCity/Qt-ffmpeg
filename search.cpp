#include "search.h"
#include "base.h"
#include "networkutil.h"
#include "searchitem.h"
#include "ui_search.h"
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
// http://cloud-music.pl-fe.cn/
Search::Search(QWidget *parent) : QWidget(parent), ui(new Ui::Search) {
  ui->setupUi(this);

  qDebug() << QSslSocket::supportsSsl(); //是否支持ssl
  qDebug() << QSslSocket::
          sslLibraryBuildVersionString(); //依赖的ssl版本
                                          //允许QTableWidget接收QWidget::customContextMenuRequested()信号。
  ui->table_playlist->setContextMenuPolicy(Qt::CustomContextMenu);
  base = new Base(ui->table_playlist);
  InitTableHeader();
  InitMenu();
  this->setWindowFlags(Qt::X11BypassWindowManagerHint |
                       Qt::FramelessWindowHint);
  InitPlayListTabWiget();

  //热搜列表
  topsearchlist = new QListWidget;
  topsearchlist->resize(355, 504);
  topsearchlist->move(280, 65);
  //底部有空白的问题
  topsearchlist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  connect(topsearchlist, &QListWidget::itemClicked, this, [&]() {
    int n = topsearchlist->currentRow();
    if (n == 0)
      return;
    GetSearchText(item.at(n - 1)->getName());
    emit clickedTopSeach();
    topsearchlist->close();
  });

  InitTopSearchItem();

  ui->scrollArea->setFrameShape(QFrame::NoFrame);

  ui->tabWidget->setWindowFlags(Qt::CustomizeWindowHint |
                                Qt::FramelessWindowHint);

  ui->table_playlist->setWindowFlags(Qt::CustomizeWindowHint |
                                     Qt::FramelessWindowHint);

  connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this,
          &Search::on_replyFinished);

  NetManager = new QNetworkAccessManager(this);

  // connect(NetManager, &QNetworkAccessManager::finished, this,
  //        [=](QNetworkReply *reply) {
  //          if (reply->error() == QNetworkReply::NoError) {
  //            QByteArray byte = reply->readAll();
  //            QPixmap map;
  //            map.loadFromData(byte);
  //          }
  //          reply->deleteLater();
  //        });
}

Search::~Search() { delete ui; }

void Search::InitTopSearchItem() {
  for (int x = 0; x != 20; ++x) {
    searchItem *items = new searchItem(topsearchlist);
    item.push_back(items);
  }
  item.at(0)->getHot()->setStyleSheet("border-image:url(:/images/HOT.png)");
}

void Search::InitPlayListTabWiget() {
  ui->tabWidget->setStyleSheet("QTabBar::tab{width:120px;height:30px;");
}
void Search::InitMenu() {
  menu = new QMenu(ui->table_playlist);
  Actplay = new QAction(QIcon(":/images/bottom/btn_play_h.png"), "播放", menu);
  Actnextplay = new QAction(QIcon(":/images/bottom/btn_single_h.png"),
                            "下一首播放", menu);
  Actdownload =
      new QAction(QIcon(":/images/btn_delete_h.png"), "删除(Delete)", menu);

  //添加入右键菜单

  menu->addAction(Actplay);
  menu->addAction(Actnextplay);
  menu->addSeparator();
  menu->addAction(Actdownload);
  //播放
  connect(Actplay, &QAction::triggered, this,
          [&] { emit play(this,ui->table_playlist->currentRow()); });
  //下一首播放
  connect(Actnextplay, &QAction::triggered, this, [&] {
    int currentRow = ui->table_playlist->currentRow();
    qDebug() << "currentRow = " << currentRow << "\n";
    emit Nextplay(this,currentRow, PlayerList.at(currentRow));
  });
  //下载
  connect(Actdownload, &QAction::triggered, this, [] {});
}

//搜索歌曲
void Search::GetSearchText(QString str) {
  ui->lab_song->setText(str);
  QString url =
      QString("http://cloud-music.pl-fe.cn/search?keywords=%1").arg(str);
  QNetworkReply *Reply = NetWorkUtil::instance()->get(url);
  typeMap.insert(Reply, RequestType::SONGTYPE);
}

void Search::Parsejson(QJsonObject &root) {
  //解析请求到的json文件
  list_id.clear();
  taglist.clear();
  PlayerList.clear();
  // QJsonParseError eeor_t{};
  // QJsonDocument deocument = QJsonDocument::fromJson(byte, &eeor_t);
  // if (eeor_t.error == QJsonParseError::NoError) {
  //  QJsonObject root = deocument.object();
  QStringList list{};
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
          tag.song_id = songsobj.value("id").toInt();
          tag.song_name = songsobj.value("name").toString();
          tag.duration = songsobj.value("duration").toInt();
          tag.mvid = songsobj.value("mvid").toInt();
          auto artists = songsobj.value("artists");
          if (artists.isArray()) {
            auto artits_ary = artists.toArray();
            auto artits = artits_ary.at(0);
            if (artits.isObject()) {
              auto artitsobj = artits.toObject();
              tag.singer_id = artitsobj.value("id").toInt();
              tag.singer_name = artitsobj.value("name").toString();
            }
          }
          //解析专辑这个json对象
          auto album = songsobj.value("album");
          if (album.isObject()) {
            auto albumobj = album.toObject();
            tag.album_id = albumobj.value("id").toInt();
            tag.album = albumobj.value("name").toString();
            //将其加入list<SearchResults>
            taglist.push_back(tag);
          }
        }
      }
    }
  }

  base->DelTableWidgetRow();
  for (int i = 0; i != taglist.count(); ++i) {
    list_id.push_back(taglist.at(i).song_id);

    QString song_name = taglist.at(i).song_name;
    QString singer_name = taglist.at(i).singer_name;
    QString album = taglist.at(i).album;
    //将时间转换为分钟和秒钟
    QString duration =
        QString("%1 : %2")
            .arg(taglist.at(i).duration / 1000 / 60, 2, 10, QChar('0'))
            .arg((int)taglist.at(i).duration % 60, 2, 10, QChar('0'));
    QStringList _list{song_name, singer_name, album, duration};
    QString url =
        QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
            .arg(taglist.at(i).song_id);
    PlayerList.push_back(url);
    base->InsertDataInfoTableWidget(_list, i);
  }
}

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

//解析获取到的热搜列表
void Search::ParseTopSearch(QJsonObject &root) {
  QJsonValue data = root.value("data");
  if (data.isArray()) {
    QJsonArray dataAry = data.toArray();
    for (auto it = dataAry.begin(); it != dataAry.end(); ++it) {
      if (dataAry.at(it.i).isObject()) {
        auto rot = dataAry.at(it.i).toObject();
        QString searchWord = rot.value("searchWord").toString();
        int score = rot.value("score").toInt();
        QString content = rot.value("content").toString();
        item.at(it.i)->setNumber(it.i + 1);
        item.at(it.i)->setName(searchWord);
        item.at(it.i)->setScore(score);
        item.at(it.i)->setContent(content);

        //设置前三项的颜色
        if (it.i < 3) {
          item.at(it.i)->setLabNumColor(true);
        } else {
          item.at(it.i)->setLabNumColor(false);
        }
        QListWidgetItem *_item = new QListWidgetItem(topsearchlist);
        _item->setSizeHint(item.at(1)->size());
        topsearchlist->setItemWidget(_item, item.at(it.i));
      }
    }
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

//歌曲详情
void Search::ParseSongDetails(QJsonObject &root) {
  // QJsonValue value = root.value("songs");
  // if (value.isArray()) {
  //  auto songary = value.toArray();
  //  QJsonValue s = songary.at(0);
  //  if (s.isObject()) {
  //    auto songobj = s.toObject();
  //    // tempTag.Title = songobj.value("name").toString();
  //    tempTag.SetTitle(songobj.value("name").toString());
  //    auto ar = songobj.value("ar");
  //    if (ar.isArray()) {
  //      auto arary = ar.toArray();
  //      /* tempTag.Artist = arary.at(0).toObject().value("name").toString();*/
  //      tempTag.SetArtist(arary.at(0).toObject().value("name").toString());
  //    }
  //    auto alVule = songobj.value("al");
  //    if (alVule.isObject()) {
  //      auto alobj = alVule.toObject();
  //      // tempTag.Ablue = alobj.value("name").toString();
  //      tempTag.SetAblue(alobj.value("name").toString());
  //      picUrl = alobj.value("picUrl").toString().toLocal8Bit();
  //      qDebug() << "pcicur" << picUrl << '\n';
  //      //获取专辑封面
  //      NetManager->get(QNetworkRequest(QString(picUrl)));
  //    }
  //  }
  //}
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
  case RequestType::SONGTYPE:
    Parsejson(root);
    ui->btn_details->setText(
        QString("歌手：%1").arg(taglist.at(0).singer_name));
    break;
  case RequestType::DETAILSTYPE:
    //获取歌曲详情
    ParseSongDetails(root);
    break;
    //热搜列表
  case RequestType::TOPSEARCHTYPE:
    ParseTopSearch(root);
    topsearchlist->show();
    break;
  default:
    break;
  }
  // reply需要我们自行delete掉，需使用deleteLater进行释放
  reply->deleteLater();
}

void Search::on_btn_playall_clicked() {
  curindex = 0;
  emit playAll(this);
}

//热搜列表
void Search::TopSearch(QWidget *wgt) {
  if (cheacktop == false) {
    cheacktop = true;
    topsearchlist->setParent(wgt);

    QLabel *lab = new QLabel("热搜榜", topsearchlist);
    QListWidgetItem *_item = new QListWidgetItem(topsearchlist);
    lab->setStyleSheet("font-size:25px");
    _item->setSizeHint(lab->size());

    //_item->setFlags(Qt::ItemIsUserCheckable);
    topsearchlist->setItemWidget(_item, lab);
    // 调用此接口,可获取热门搜索列表
    static QString url{"http://cloud-music.pl-fe.cn/search/hot/detail"};
    QNetworkReply *Reply = NetWorkUtil::instance()->get(url);
    typeMap.insert(Reply, RequestType::TOPSEARCHTYPE);
  } else {
    topsearchlist->show();
    return;
  }
}

void Search::on_table_playlist_cellDoubleClicked(int row, int column) {
  Q_UNUSED(column);
  curindex = row;
  int id = taglist.at(row).song_id;
  QNetworkReply *Reply = NetWorkUtil::instance()->get(
      QString("http://cloud-music.pl-fe.cn/check/music?id=%1").arg(id));
  typeMap.insert(Reply, RequestType::STATETYPE);
}

//下载全部
void Search::on_btn_downloadall_clicked() {}

void Search::on_table_playlist_customContextMenuRequested(const QPoint &pos) {
  if (ui->table_playlist->itemAt(pos) != nullptr) {
    menu->exec(QCursor::pos());
  }
}