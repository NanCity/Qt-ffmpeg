#include "search.h"
#include "base.h"
#include "networkutil.h"
#include "searchitem.h"
#include "tag.h"
#include "ui_search.h"
#include <QByteArray>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QListWidget>
#include <QMessageBox>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
#include <QScrollBar>
// http://localhost:3000/
Search::Search(QWidget* parent) : QWidget(parent), ui(new Ui::Search) {
	ui->setupUi(this);
	base = new Base(ui->table_playlist);
	mutex = new QMutex();
	Netstatus = new NetworkStatus;
	connect(Netstatus, &NetworkStatus::timeout, this, []() { return; });
	InitMenu();

	qDebug() << QSslSocket::supportsSsl();                  //是否支持ssl
	qDebug() << QSslSocket::sslLibraryBuildVersionString(); //依赖的ssl版本

	InitTableHeader();

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
	NetSinger = new QNetworkAccessManager(this);
	NetSingetPic = new QNetworkAccessManager(this);

	connect(NetSinger, &QNetworkAccessManager::finished, this,
		&Search::on_finshedSinger);

	connect(NetSingetPic, &QNetworkAccessManager::finished, this,
		[&](QNetworkReply* reply) {
			if (reply->error() == QNetworkReply::NoError) {
				singerpic.loadFromData(reply->readAll());
			}
			qDebug() << "signer pic\n";
			reply->deleteLater();
		});

	// connect(NetManager, &QNetworkAccessManager::finished, this,
	//        [=](QNetworkReply *reply) {
	//          if (reply->error() == QNetworkReply::NoError) {
	//            QByteArray byte = reply->readAll();
	//            QPixmap map;
	//            map.loadFromData(byte);
	//          }
	//          reply->deleteLater();
	//        });
	//选项卡被点击
	connect(ui->tabWidget, &QTabWidget::tabBarClicked, this,
		&Search::on_tabClicked);
}

Search::~Search() {
	delete ui;
	delete mutex;
	mutex = nullptr;
	delete Netstatus;
	Netstatus = nullptr;
}

void Search::InitTopSearchItem() {
	for (int x = 0; x != 20; ++x) {
		searchItem* items = new searchItem(topsearchlist);
		item.push_back(items);
	}
	item.at(0)->getHot()->setStyleSheet("border-image:url(:/images/HOT.png)");
}

void Search::InitPlayListTabWiget() {
	ui->tabWidget->setStyleSheet("QTabBar::tab{width:120px;height:30px;");
}

void Search::InitMenu() {
	//评论
	connect(base->Actcomment, &QAction::triggered, this,
		[&] { base->Actcomment->setText("评论999+"); });

	//播放当前点击的歌曲
	connect(base->Actplay, &QAction::triggered, this, [&] {
		int current = ui->table_playlist->currentRow();
		emit play(this, current);
		});

	//下一首播放
	connect(base->Actnextplay, &QAction::triggered, this, [&] {
		int curent = ui->table_playlist->currentRow();
		emit Nextplay(this, curent, songId.at(curent));
		});

	//收藏
	connect(base->Actcollect, &QAction::triggered, this, [&] {});
	//下载
	connect(base->Actcollect, &QAction::triggered, this, [&] {});
}
//搜索歌曲
void Search::GetSearchText(QString _str) {
	base->loadMovie();
	this->str = _str;
	ui->lab_song->setText(_str);
	QString url =
		QString("http://localhost:3000/search?keywords=%1&limit=100")
		.arg(_str);
	QNetworkReply* Reply = NetWorkUtil::instance()->get(url);
	typeMap.insert(Reply, RequestType::SONGTYPE);
}

void Search::Parsejson(QJsonObject& root) {
	//解析请求到的json文件
	taglist.clear();
	songId.clear();
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
					tag.song_id = songsobj.value("id").toInt();
					songId.push_back(QString::number(tag.song_id));
					tag.song_name = songsobj.value("name").toString();
					int dur = songsobj.value("duration").toInt();
					tag.duration = QString("%1 : %2")
						.arg(dur / 1000 / 60, 2, 10, QChar('0'))
						.arg(dur % 60, 2, 10, QChar('0'));

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
					tag.status = songsobj.value("status").toInt();
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
		loadData();
	}
}

void Search::InitTableHeader() {
	QStringList HorizontalHeaderItem{
		QObject::tr("操作"), QObject::tr("音乐标题"), QObject::tr("歌手"),
		QObject::tr("专辑"), QObject::tr("时长"),
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
	ui->table_playlist->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}

//解析获取到的热搜列表
void Search::ParseTopSearch(QJsonObject& root) {
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
				}
				else {
					item.at(it.i)->setLabNumColor(false);
				}
				QListWidgetItem* _item = new QListWidgetItem(topsearchlist);
				_item->setSizeHint(item.at(1)->size());
				topsearchlist->setItemWidget(_item, item.at(it.i));
			}
		}
	}
}

void Search::NetWorkState(QNetworkReply* reply) {
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


void Search::on_replyFinished(QNetworkReply* reply) {
	if (reply->error() != QNetworkReply::NoError) {
		qDebug() << reply->errorString();
		NetWorkState(reply);
		return;
	}
	QByteArray byte{ reply->readAll() };
	QJsonParseError eeor_t{};
	QJsonObject root{};
	QJsonDocument deocument = QJsonDocument::fromJson(byte, &eeor_t);
	if (eeor_t.error != QJsonParseError::NoError) {
		QMessageBox::warning(this, tr("warning"), tr("Json format error\n"),
			QMessageBox::YesAll);
		ERROR("Jsong format error");
		return;
	}
	else {
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
void Search::TopSearch(QWidget* wgt) {
	if (cheacktop == false) {
		cheacktop = true;
		topsearchlist->setParent(wgt);

		QLabel* lab = new QLabel("热搜榜", topsearchlist);
		QListWidgetItem* _item = new QListWidgetItem(topsearchlist);
		lab->setStyleSheet("font-size:25px");
		_item->setSizeHint(lab->size());

		//_item->setFlags(Qt::ItemIsUserCheckable);
		topsearchlist->setItemWidget(_item, lab);
		// 调用此接口,可获取热门搜索列表
		static QString url{ "http://localhost:3000/search/hot/detail" };
		QNetworkReply* Reply = NetWorkUtil::instance()->get(url);
		typeMap.insert(Reply, RequestType::TOPSEARCHTYPE);
	}
	else {
		topsearchlist->show();
		return;
	}
}

void Search::loadData() {
	int index = 0;
	//重新定位到开始处
	ui->table_playlist->verticalScrollBar()->setValue(0);
	foreach(const NetSongTag & rhs, taglist) {
		ui->table_playlist->insertRow(index);
		if (base->isLike(rhs.song_id)) {
			ui->table_playlist->setCellWidget(index, 0, base->setItemWidget(1));
		}
		else
		{
			ui->table_playlist->setCellWidget(index, 0, base->setItemWidget(0));
		}
		QTableWidgetItem* item1 = new QTableWidgetItem(rhs.singer_name);
		QTableWidgetItem* item2 = new QTableWidgetItem(rhs.song_name);
		QTableWidgetItem* item3 = new QTableWidgetItem(rhs.album);
		QTableWidgetItem* item4 = new QTableWidgetItem(rhs.duration);
		ui->table_playlist->setItem(index, 1, item1);
		ui->table_playlist->setItem(index, 2, item2);
		ui->table_playlist->setItem(index, 3, item3);
		ui->table_playlist->setItem(index, 4, item4);
		++index;
	}
	base->closeMovie();
}

void Search::on_table_playlist_cellDoubleClicked(int row, int column) {
	Q_UNUSED(column);
	curindex = row;
	int id = taglist.at(row).song_id;
}

//下载全部
void Search::on_btn_downloadall_clicked() {}

void Search::on_tabClicked(int index) {
	switch (index) {
		// 0
		// 默认显示单曲,由于c++规定：不允许跨过变量的初始化语句直接跳转到该变量作用域的另一个位置
		//所以这里把变量定义在块内，解决初始化问题
	case 1: {
		//歌手 item
		QString url{
			QString("http://localhost:3000/cloudsearch?keywords=%1&type=100")
				.arg(str) };
		NetSinger->get(QNetworkRequest(url));
	} break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	default:
		break;
	}
}


void Search::on_finshedSinger(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			SingerIdList.clear();
			QJsonObject rot = document.object();
			QJsonObject resultobj = rot.value("result").toObject();
			QJsonValue art = resultobj.value("artists");
			if (art.isArray()) {
				Singerlist.clear();
				ui->listwidg_song->clear();
				QJsonArray artAry = art.toArray();
				foreach(const QJsonValue & rhs, artAry) {
					if (rhs.isObject()) {
						QJsonObject obj = rhs.toObject();
						Singer singer;
						singer.setid(obj.value("id").toInt());
						singer.setname(obj.value("name").toString());
						singer.setpicUrl(obj.value("picUrl").toString());

						//启动一个事件循环，finshed执行完毕以后再往下
						QEventLoop loop;
						NetSingetPic->get(QNetworkRequest(singer.getpicUrl()));
						connect(NetSingetPic, &QNetworkAccessManager::finished, &loop,
							&QEventLoop::quit);
						loop.exec();

						QJsonValue alia = obj.value("alia");
						if (alia.isArray()) {
							QJsonArray aliasAry = alia.toArray();
							singer.setalias(aliasAry.at(0).toString());
						}
						SingerIdList.push_back(singer);
						QWidget* widget = new QWidget(this);
						QLabel* pic = new QLabel(widget);
						//填充图片
						pic->setScaledContents(true);
						pic->setPixmap(singerpic);
						pic->setMaximumWidth(60);
						pic->setMaximumHeight(60);
						// pic->setMinimumSize(QSize(60, 60));
						QPushButton* btn = new QPushButton(singer.getname(), widget);
						btn->setMinimumSize(QSize(100, 30));
						QHBoxLayout* hbox = new QHBoxLayout(ui->listwidg_song);
						hbox->addWidget(pic);
						hbox->addWidget(btn);
						//边距
						hbox->setSpacing(3);
						//右边添加一个弹簧
						hbox->addStretch();
						widget->setLayout(hbox);
						QListWidgetItem* item = new QListWidgetItem(ui->listwidg_song);
						item->setSizeHint(QSize(ui->listwidg_song->width(), 80));

						ui->listwidg_song->setItemWidget(item, widget);
						Singerlist.push_back(singer);
					}
				}
			}
		}
	}
	reply->deleteLater();
}
//许嵩ID = 5771
void Search::on_listwidg_song_itemClicked(QListWidgetItem* item) {
	int index = ui->listwidg_song->currentRow();
	QString Url{ QString("http://localhost:3000/artist/detail?id=%1").arg(SingerIdList.at(index).getid()) };
	//NetSingerDetails->get(QNetworkRequest(Url));
	emit getid(SingerIdList.at(index).getid());
	//获取里面的QWidget
	//QWidget* widget = ui->listwidg_song->itemWidget(item);
	//if (widget != nullptr) {
	//	//拿到item里面的QPushButton按钮
	//	QPushButton* btn = widget->findChild<QPushButton*>();
	//	QString str = btn->text();
	//	QString Url{QString("http://localhost:3000/artist/detail?id=").arg("")};
	//}

}
