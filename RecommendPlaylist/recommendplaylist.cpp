#include "recommendplaylist.h"
#include "recommendplaylist.h"
#include "recommendplaylist.h"
#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "recommendplaylist.h"
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
//水平弹簧
#include <QSpacerItem>
#include <QScrollArea>
//获取推荐歌单
//http://cloud-music.pl-fe.cn/top/playlist?limit=10&order=new

RecommendPlaylist::RecommendPlaylist(QWidget* parent) : QWidget(parent),
ui(new Ui::RecommendPlaylist)
{
	ui->setupUi(this);

	playlistGrid = new QGridLayout(ui->widget);
	ui->btn_Boutique_playlist->setStyleSheet("background-color:#5e8149");
	Bout_pic = new QLabel(ui->btn_Boutique_playlist);
	Bout_pic->setScaledContents(true);
	Bout_pic->setGeometry(18, 18, 140, 140);
	Bout_pic->setStyleSheet("background-color:red");

	btn_2 = new QPushButton(QIcon(":/images/crown.png"), "精品歌单", ui->btn_Boutique_playlist);
	btn_2->setGeometry(160, 50, 105, 35);
	btn_2->setStyleSheet("background-repeat:no-repeat;text-align:left;"
		"padding-left:20px;background-color:transparent;color:#e7aa5a");

	//精品歌单描述
	lab = new QLabel("测试测试", ui->btn_Boutique_playlist);
	lab->setGeometry(160, 100, 510, 30);
	lab->setStyleSheet("color:white;font-size: 18px");

	NetGetReclist = new QNetworkAccessManager(this);
	NetGetPic = new QNetworkAccessManager(this);
	NetBoutPlaylist = new QNetworkAccessManager(this);
	connect(NetGetReclist, &QNetworkAccessManager::finished, this, &RecommendPlaylist::on_finsgedNetGetReclist);
	connect(NetGetPic, &QNetworkAccessManager::finished, this, &RecommendPlaylist::on_finshedNetGetPic, Qt::UniqueConnection);
	connect(NetBoutPlaylist, &QNetworkAccessManager::finished, this, &RecommendPlaylist::on_finshedNetBoutPlaylist);
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
		QString URL{ QString("http://cloud-music.pl-fe.cn/top/playlist?cat=%1&limit=30").arg("华语") };
		NetGetReclist->get(QNetworkRequest(URL));
		//获取首页精品歌单封面
		QString URL_Bout{ QString("http://cloud-music.pl-fe.cn/top/playlist/highquality?cat=%1").arg("全部歌单") };
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


void RecommendPlaylist::on_btn_Boutique_playlist_clicked()
{
	index = 1;
	ui->stackedWidget->setCurrentIndex(1);
	getBoutPlaylist("全部歌单");
}

void RecommendPlaylist::getBoutPlaylist(const QString& str)
{
	QString URL{ QString("http://cloud-music.pl-fe.cn/top/playlist/highquality?cat=%1").arg(str) };
	NetBoutPlaylist->get((QNetworkRequest(URL)));
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
				NetGetPic->get(QNetworkRequest(list.coverImgUrl));
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
	}
}

void RecommendPlaylist::on_finsgedNetGetReclist(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			paseJosn(obj, "playlists", playlists);
		}
	}
	reply->deleteLater();
}

void RecommendPlaylist::on_finshedNetGetPic(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QPixmap pix;
		pix.loadFromData(reply->readAll());
		if (index == 0) {
			index = 0;
			addPlaylsit(ui->widget, pix);
		}
		else
		{
			addBoutPlaylist(ui->stackedWidget->widget(1), pix);
			index = 1;
		}

	}

	reply->deleteLater();
}

void RecommendPlaylist::on_finshedNetBoutPlaylist(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			paseJosn(obj, "playlists", bout_playlist);
		}
	}
	reply->deleteLater();
}

void RecommendPlaylist::on_btnclicked()
{
	QObject* obj = sender();
	auto x = btn_id.find(obj->objectName());
	size_t id = x.value().id;
	int limit = x.value().trackCount;
	emit getSongMenu(id, limit);
}


void RecommendPlaylist::addPlaylsit(QWidget* wid, const QPixmap& pix) {
	QVBoxLayout* vbox = new QVBoxLayout(wid);
	QString objname{};
	static int row = 0, colum = 0, i = 0;
	QPushButton* btn = new QPushButton(wid);
	objname = QString("btn_%1").arg(i);
	btn->setObjectName(objname);
	btn->setMaximumSize(180, 180);
	btn->setMinimumSize(180, 180);
	btn->setIconSize(QSize(180, 180));
	btn->setIcon(pix);

	//btn内部title
	QLabel* title = new QLabel(playlists.at(i).nickname, btn);
	title->setStyleSheet("color:#ffffff");
	title->setGeometry(10, 160, 110, 20);
	vbox->addWidget(btn);

	QLabel* lab = new QLabel(wid);
	lab->setWordWrap(true);
	lab->setMaximumSize(180, 25);
	lab->setMinimumSize(180, 25);
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

void RecommendPlaylist::addBoutPlaylist(QWidget* wid, const QPixmap& pix)
{
	static int row = 0, colum = 0, i = 0;
	static QGridLayout* widGrid = new QGridLayout(ui->Bout_widget);
	QVBoxLayout* vbox = new QVBoxLayout(ui->Bout_widget);
	QHBoxLayout* hbox = new QHBoxLayout(ui->Bout_widget);

	QPushButton* btn = new QPushButton(ui->Bout_widget);
	btn->setMaximumSize(135, 135);
	btn->setMinimumSize(135, 135);
	btn->setIconSize(btn->size());
	btn->setIcon(pix);
	hbox->addWidget(btn);

	hbox->setContentsMargins(0, 0, 0, 0);

	QLabel* lab_1 = new QLabel(bout_playlist.at(i).name, ui->Bout_widget);
	lab_1->setMaximumSize(250, 20);
	lab_1->setMinimumSize(250, 20);
	QLabel* lab_2 = new QLabel(bout_playlist.at(i).nickname, ui->Bout_widget);
	lab_2->setMaximumSize(250, 20);
	lab_2->setMinimumSize(250, 20);

	vbox->addWidget(lab_1);
	vbox->addWidget(lab_2);
	hbox->addLayout(vbox);

	if (i > 0 && i % 3 == 0) {
		++row; colum = 0;
		widGrid->addLayout(hbox, row, colum, 1, 1);
	}
	widGrid->addLayout(hbox, row, colum, 1, 1);

	widGrid->setContentsMargins(0, 0, 0, 0);
	ui->Bout_widget->setLayout(widGrid);
	++i;
	++colum;
	if (i == bout_playlist.length()) {
		row = 0; colum = 0; i = 0;
	}
}
