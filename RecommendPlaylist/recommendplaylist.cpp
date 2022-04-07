#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "recommendplaylist.h"
#include "ui_recommendplaylist.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEventLoop>
//水平弹簧
#include <QSpacerItem>
#include <QScrollArea>
#include <QMutex>
//获取推荐歌单
//http://localhost:3000/top/playlist?limit=10&order=new

RecommendPlaylist::RecommendPlaylist(QWidget* parent) : QWidget(parent),
ui(new Ui::RecommendPlaylist)
{
	ui->setupUi(this);
	MangerRecPlaylist = new QNetworkAccessManager(this);

	playlistGrid = new QGridLayout(ui->widget);

	//精品歌单ui布局
	scrollAreaWidgetContents_3_Grid = new QGridLayout(ui->scrollAreaWidgetContents_3);
	//顶部的标题先添加进来
	scrollAreaWidgetContents_3_Grid->addLayout(ui->horizontalLayout_3, 0, 0,1,3);

	ui->btn_Boutique_playlist->setStyleSheet("background-color:#5e8149");
	Bout_pic = new QLabel(ui->btn_Boutique_playlist);
	Bout_pic->setScaledContents(true);
	Bout_pic->setGeometry(18, 18, 140, 140);

	//Bout_pic->setStyleSheet("background-color:red");

	btn_2 = new QPushButton(QIcon(":/images/crown.png"), "精品歌单", ui->btn_Boutique_playlist);
	btn_2->setGeometry(160, 50, 105, 35);
	btn_2->setStyleSheet("background-repeat:no-repeat;text-align:left;"
		"padding-left:20px;background-color:transparent;color:#e7aa5a");

	//精品歌单描述
	lab = new QLabel(ui->btn_Boutique_playlist);
	lab->setGeometry(160, 100, 510, 30);
	lab->setStyleSheet("color:white;font-size: 18px");


	connect(NetGetPic, &QNetworkReply::finished, this, &RecommendPlaylist::on_finshedNetGetPic, Qt::UniqueConnection);
}

RecommendPlaylist::~RecommendPlaylist()
{
	delete ui;
}

/*
* cat = tag, 比如 " 华语 "、" 古风 " 、" 欧美 "、" 流行 "
* limit = 30 一次取30个
*/
void RecommendPlaylist::getRecPlayList()
{
	ui->stackedWidget->setCurrentIndex(0);
	//第一次点击
	if (first == true) {
		playlists.clear();
		btn_id.clear();
		// /top/playlist?limit=10&order=new
		QString URL{ QString("http://localhost:3000/top/playlist?cat=%1&limit=100").arg("华语") };
		//获取推荐歌单
		NetGetReclist = MangerRecPlaylist->get(QNetworkRequest(URL));
		connect(NetGetReclist, &QNetworkReply::finished, this, &RecommendPlaylist::on_finsgedNetGetReclist);
		//获取首页精品歌单封面
		QString URL_Bout{ QString("http://localhost:3000/top/playlist/highquality?cat=%1").arg("全部歌单") };
		QNetworkAccessManager* boutplist = new QNetworkAccessManager(this);
		boutplist->get(QNetworkRequest(URL_Bout));
		connect(boutplist, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
			if (reply->error() == QNetworkReply::NoError) {
				QJsonParseError err_t{};
				QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err_t);
				if (err_t.error == QJsonParseError::NoError) {
					QJsonObject obj = doc.object();
					QJsonValue val = obj.value("playlists");
					if (val.isArray()) {
						QJsonArray ary = val.toArray();
						lab->setText(ary.at(0).toObject().value("name").toString());
						QString picUtr = ary.at(0).toObject().value("coverImgUrl").toString();
						QNetworkAccessManager* pic = new QNetworkAccessManager(this);
						pic->get(QNetworkRequest(picUtr));
						connect(pic, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
							if (reply->error() == QNetworkReply::NoError) {
								QPixmap pix{};
								pix.loadFromData(reply->readAll());
								Bout_pic->setPixmap(pix);
							}
							});
					}
				}
			}
			reply->deleteLater();
			});
		first = false;
	}
}


//获取精品歌单
void RecommendPlaylist::on_btn_Boutique_playlist_clicked()
{
	index = 1;
	ui->stackedWidget->setCurrentIndex(1);
	getBoutPlaylist("全部歌单");
}

void RecommendPlaylist::getBoutPlaylist(const QString& str)
{
	QString URL{ QString("http://localhost:3000/top/playlist/highquality?cat=%1").arg(str) };
	NetBoutPlaylist = MangerRecPlaylist->get(QNetworkRequest(URL));
	connect(NetBoutPlaylist, &QNetworkReply::finished, this, &RecommendPlaylist::on_finshedNetBoutPlaylist);
}


void RecommendPlaylist::paseJosn(QJsonObject& obj, const QString& str, QList<Playlists>& _playlists)
{
	Playlists list{};
	QJsonValue val = obj.value("playlists");
	if (val.isArray()) {
		QJsonArray ary = val.toArray();
		foreach(const QJsonValue & rhs, ary) {
			if (rhs.isObject()) {
				QJsonObject obj = rhs.toObject();
				list.coverImgUrl = obj.value("coverImgUrl").toString();
				//获取专辑封面
			/*	NetGetPic  = MangerRecPlaylist->get(QNetworkRequest(list.coverImgUrl));*/
				list.name = obj.value("name").toString();
				list.id = obj.value("id").toVariant().toULongLong();
				list.playCount = obj.value("playCount").toVariant().toULongLong();
				list.trackCount = obj.value("trackCount").toInt();
				//获取歌单创建者信息
				QJsonValue val = obj.value("creator");
				if (val.isObject()) {
					//707
					QJsonObject obj_2 = val.toObject();
					list.nickname = obj_2.value("nickname").toString();
					QString avatar = obj_2.value("avatarDetail").toString();
					if (avatar == "null") {
						list.avatarDetail = false;
					}
				}
				_playlists.push_back(list);
			}
		}
		th = std::thread{
		[&]() {
			foreach(const Playlists & str,_playlists) {
			NetCoverImg = MangerRecPlaylist->get(QNetworkRequest(str.coverImgUrl));
			QEventLoop  loop{};
			connect(NetCoverImg, &QNetworkReply::finished, this, &RecommendPlaylist::on_finshedNetCoverImg);
			connect(NetCoverImg, &QNetworkReply::finished, &loop, &QEventLoop::quit);
			//不处理套接字通知事件
			loop.exec(QEventLoop::ExcludeUserInputEvents);
		}}
		};
		th.detach();
	}
}

void RecommendPlaylist::on_finsgedNetGetReclist()
{
	if (NetGetReclist->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(NetGetReclist->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			paseJosn(obj, "playlists", playlists);
		}
	}
}

void RecommendPlaylist::on_finshedNetGetPic()
{
	if (NetGetPic->error() == QNetworkReply::NoError) {
		QPixmap pix;
		pix.loadFromData(NetGetPic->readAll());
		if (index == 0) {
			index = 0;
			addPlaylsit(ui->widget, pix);
		}
		else
		{
			//addBoutPlaylist(ui->stackedWidget->widget(1), pix);
			index = 1;
		}

	}
}

void RecommendPlaylist::on_finshedNetBoutPlaylist()
{
	if (NetBoutPlaylist->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(NetBoutPlaylist->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			//解析精品歌单
			paseBoutiquePlaylistJson(obj);
		}
	}
}

void RecommendPlaylist::paseBoutiquePlaylistJson(const QJsonObject& obj)
{
	btn_clicked_id = 0;
	QJsonValue val = obj.value("playlists");
	Playlists boutique{};
	if (val.isArray()) {
		QJsonArray ary = val.toArray();
		foreach(const QJsonValue rhs, ary) {
			if (rhs.isObject()) {
				QJsonObject obj = rhs.toObject();
				boutique.name = obj.value("name").toString();
				boutique.id = obj.value("id").toVariant().toULongLong();
				boutique.playCount = obj.value("playCount").toVariant().toULongLong();
				boutique.trackCount = obj.value("trackCount").toInt();
				boutique.coverImgUrl = obj.value("coverImgUrl").toString();
				if (obj.value("creator").isObject()) {
					QJsonObject subobj = obj.value("creator").toObject();
					boutique.nickname = subobj.value("nickname").toString();
				}

				if (obj.value("avatarDetail").toString().isEmpty()) {
					boutique.avatarDetail = true;
				}
			}
			bout_playlist.push_back(boutique);
		}
		botuiqueThread = std::thread([&]() {
			QEventLoop loop{};
			//foreach(const Playlists& rhs, bout_playlist)
			for (const auto& rhs : bout_playlist)
			{
				NetBoutImage = MangerRecPlaylist->get(QNetworkRequest(rhs.coverImgUrl));
				connect(NetBoutImage, &QNetworkReply::finished, this,
					&RecommendPlaylist::on_finshedNetBoutImage, Qt::QueuedConnection);

				connect(NetBoutImage, &QNetworkReply::finished, &loop, &QEventLoop::quit);
				loop.exec(QEventLoop::ExcludeUserInputEvents);
			}
			});
		botuiqueThread.detach();
	}
}

void RecommendPlaylist::on_finshedNetBoutImage()
{
	if (NetBoutImage->error() == QNetworkReply::NoError) {
		QPixmap pix{};
		pix.loadFromData(NetBoutImage->readAll());
		addBoutPlaylist(pix);
	}
}

void RecommendPlaylist::on_finshedNetCoverImg()
{
	if (NetCoverImg->error() == QNetworkReply::NoError) {
		QPixmap pix{};
		QByteArray byte{ NetCoverImg->readAll() };
		pix.loadFromData(byte);
		addPlaylsit(ui->widget, pix);
	}
}


void RecommendPlaylist::on_btnclicked()
{
	QObject* obj = sender();
	auto x = btn_id.find(obj->objectName());
	size_t id = x.value().id;
	int limit = x.value().trackCount;
	emit getSongMenu(id, limit);
}

void RecommendPlaylist::addPlaylsit(QWidget* wid, const QPixmap pix) {
	QVBoxLayout* vbox = new QVBoxLayout(wid);
	QString objname{};
	static int row = 0, colum = 0, i = 0;
	QPushButton* btn = new QPushButton(wid);
	objname = QString("btn_%1").arg(i);
	btn->setIconSize(QSize(180, 180));
	btn->setIcon(pix);
	btn->setObjectName(objname);
	btn->setMaximumSize(180, 180);
	btn->setMinimumSize(180, 180);

	//btn内部title
	QLabel* title = new QLabel(playlists.at(i).nickname, btn);
	title->setStyleSheet("color:#ffffff");
	title->setGeometry(10, 160, 110, 20);
	vbox->addWidget(btn);

	QLabel* lab = new QLabel(wid);
	lab->setWordWrap(true);
	lab->setMaximumSize(180, 30);
	lab->setMinimumSize(180, 30);
	vbox->addWidget(lab);
	lab->setText(playlists.at(i).name);

	if (i % 5 == 0 && i > 0) {
		//换行
		++row;
		colum = 0;
		playlistGrid->addLayout(vbox, row, colum, 1, 1);
	}
	playlistGrid->addLayout(vbox, row, colum, 1, 1);

	connect(btn, &QPushButton::clicked, this, &RecommendPlaylist::on_btnclicked);
	//保存id与btn的映射关系
	btn_id.insert(objname, playlists.at(i));
	wid->setLayout(playlistGrid);
	++colum;
	++i;
	if (i == playlists.length()) {
		row = 0; colum = 0; i = 0;
	}
}

void RecommendPlaylist::addBoutPlaylist(const QPixmap& pix)
{
	static int x = 1, y = 0;
	QPushButton* btn_image = new QPushButton(ui->scrollAreaWidgetContents_3);
	QString objectName = QString("btn_%1").arg(btn_clicked_id);
	btn_image->setObjectName(objectName);
	btn_image->setMinimumSize(130, 135);
	btn_image->setIconSize(btn_image->size());
	btn_image->setIcon(QIcon(pix));
	connect(btn_image, &QPushButton::clicked, this, &RecommendPlaylist::on_btn_clicked);

	//垂直布局
	QVBoxLayout* vbox = new QVBoxLayout();
	QLabel* lab_title = new QLabel(ui->scrollAreaWidgetContents_3);
	lab_title->setText(bout_playlist.at(btn_clicked_id).name);
	lab_title->setMaximumSize(175, 25);
	vbox->addWidget(lab_title);

	QLabel* lab_artist = new QLabel(ui->scrollAreaWidgetContents_3);
	lab_artist->setText(bout_playlist.at(btn_clicked_id).nickname);
	lab_artist->setMaximumSize(175, 25);
	vbox->addWidget(lab_artist);
	//水平布局
	QHBoxLayout* hbox = new QHBoxLayout();
	hbox->addWidget(btn_image);
	hbox->addLayout(vbox);

	btn_botuique_id.insert(objectName, bout_playlist.at(btn_clicked_id));
	if (y == 3) {
		y = 0;
		++x;
	}
	scrollAreaWidgetContents_3_Grid->addLayout(hbox, x, y, 1, 1);
	++y;
	++btn_clicked_id;
}


void RecommendPlaylist::on_btn_clicked()
{
	//获取放出信号的按钮
	QObject* btn = sender();
	auto x = btn_botuique_id.find(btn->objectName());
	//打开专辑UI
	emit getSongMenu(x.value().id, x.value().trackCount);
}
