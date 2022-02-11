#include "songmenu.h"
#include "ui_songmenu.h"
#include "base.h"
#include "tag.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>
#include <QScrollBar>
#include <QThread>
SongMenu::SongMenu(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::SongMenu)
{
	ui->setupUi(this);
	base = new Base(ui->tab_SongTable);
	NetSongMenu = new QNetworkAccessManager(this);
	NetAllSong = new QNetworkAccessManager(this);
	//设置第一列表头自适应widget宽高
	ui->tab_SongTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	connect(NetSongMenu, &QNetworkAccessManager::finished, this, &SongMenu::on_finshedNetSongMenu);
	connect(NetAllSong, &QNetworkAccessManager::finished, this, &SongMenu::on_finsedNetAllSong);
	//获取用户歌单
	RequestUserSongMenu();
	//加载剩余的数据
	connect(base, &Base::loadNextPage, this, [&]() {
		for (int i = 0; i != 5; i++) {
			if (curtableindex >= taglsit.length()) {
				break;
			}
			else
			{
				int currow = ui->tab_SongTable->rowCount();
				ui->tab_SongTable->insertRow(currow);
				if (base->isLike(taglsit.at(currow).Songid)) {
					ui->tab_SongTable->setCellWidget(currow, 0, base->setItemWidget(1));
				}
				else
				{
					ui->tab_SongTable->setCellWidget(currow, 0, base->setItemWidget(0));
				}

				QTableWidgetItem* item1 = new QTableWidgetItem(taglsit.at(currow).Title);
				QTableWidgetItem* item2 = new QTableWidgetItem(taglsit.at(currow).Artist);
				QTableWidgetItem* item3 = new QTableWidgetItem(taglsit.at(currow).Ablue);
				QTableWidgetItem* item4 = new QTableWidgetItem(taglsit.at(currow).Duration);
				ui->tab_SongTable->setItem(currow, 1, item1);
				ui->tab_SongTable->setItem(currow, 2, item2);
				ui->tab_SongTable->setItem(currow, 3, item3);
				ui->tab_SongTable->setItem(currow, 4, item4);
				++curtableindex;
			}
		}
		});

}

SongMenu::~SongMenu()
{
	delete ui;
}


void SongMenu::RequestUserSongMenu()
{
	QString userid = config.GetValue("/Userinfo/userId");
	if (userid.isEmpty()) {
		QLabel* lab = new QLabel(ui->tab_SongTable);
		lab->setText("还未登录账号");
		lab->move(ui->tab_SongTable->width() / 2 + 10, 40);
		return;
	}
	else
	{
		QString Url{ QString("http://cloud-music.pl-fe.cn/user/playlist?uid=%1").arg(userId) };
		QNetworkRequest* req = config.setCookies();
		req->setUrl(Url);
		NetSongMenu->get(*req);
	}
}
void SongMenu::SongMenuAt(const int index)
{
	base->loadMovie();
	base->DelTableWidgetRow();
	QString Url{ QString("http://localhost:3000/playlist/track/all?id=%1&limit=%2")
		.arg(songlistMenu.at(index).id).arg(songlistMenu.at(index).trackCount) };
	QNetworkRequest* request{ config.setCookies() };
	request->setUrl(Url);
	NetAllSong->get(*request);
}

/*
* id = 歌单ID
* limit = 每次加载的数量
*/
void SongMenu::getSongMenuID(const long long ID, const int limit)
{
	base->loadMovie();
	emit DataLoading();
	QString Url{ QString("http://localhost:3000/playlist/track/all?id=%1&limit=%2").arg(ID).arg(limit) };
	qDebug() << "获取所有的歌曲\nURL = " << Url;
	NetAllSong->get(QNetworkRequest(Url));
}

void SongMenu::CreatorSongMuen(const QString& str)
{
	if (str.isEmpty())return;
	QString URL{ QString("http://cloud-music.pl-fe.cn/playlist/create?name=%1").arg(str) };
	QNetworkRequest* req{ config.setCookies() };
	QNetworkAccessManager* Creat = new QNetworkAccessManager(this);
	req->setUrl(URL);
	Creat->get(*req);
	connect(Creat, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
		if (reply->error() == QNetworkReply::NoError) {
			QJsonParseError err_t{};
			QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
			if (err_t.error == QJsonParseError::NoError) {
				QJsonObject obj = document.object();
				UserSongMuen menu;
				menu.id = obj.value("id").toInt();
				menu.trackCount = 0;
				songlistMenu.push_back(menu);
				emit CreatorSongMenuOk();
			}
			reply->deleteLater();
		}
		});
}

void SongMenu::DelereSongMuenu(const int ID)
{
	QString URL{ "http://cloud-music.pl-fe.cn/playlist/delete?id=" + ID };
	QNetworkAccessManager* del = new QNetworkAccessManager(this);
	QNetworkRequest* req{ config.setCookies() };
	del->get(*req);
	connect(del, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
		if (reply->error() == QNetworkReply::NoError) {
			QJsonParseError err_t{};
			QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
			if (err_t.error == QJsonParseError::NoError) {
				QJsonObject obj = document.object();
				if (obj.value("message").toBool() == false) {
					fprintf(stdout, "歌单删除失败\n");
				}
			}
		}
		reply->deleteLater();
		});
}



void SongMenu::loadData()
{
	int len = taglsit.length();
	for (int i = 0; i != 20; ++i) {
		//越界检查
		if (i >= len) {
			curtableindex = i;
			base->closeMovie();
			return;
		}

		ui->tab_SongTable->insertRow(i);
		if (base->isLike(taglsit.at(i).Songid)) {
			ui->tab_SongTable->setCellWidget(i, 0, base->setItemWidget(1));
		}
		else
		{
			ui->tab_SongTable->setCellWidget(i, 0, base->setItemWidget(0));
		}

		QTableWidgetItem* item1 = new QTableWidgetItem(taglsit.at(i).Title);
		QTableWidgetItem* item2 = new QTableWidgetItem(taglsit.at(i).Artist);
		QTableWidgetItem* item3 = new QTableWidgetItem(taglsit.at(i).Ablue);
		QTableWidgetItem* item4 = new QTableWidgetItem(taglsit.at(i).Duration);
		ui->tab_SongTable->setItem(i, 1, item1);
		ui->tab_SongTable->setItem(i, 2, item2);
		ui->tab_SongTable->setItem(i, 3, item3);
		ui->tab_SongTable->setItem(i, 4, item4);
	}
	base->closeMovie();
	//第一次加载数量
	curtableindex = 20;
	//下面的加载大量数据会卡顿，原因:一次性把所有的数据都添加了
	/*foreach(const Temptag & rhs, taglsit) {
		ui->tab_SongTable->insertRow(index);
		ui->tab_SongTable->setCellWidget(index, 0, base->setItemWidget(1));
		QTableWidgetItem* item1 = new QTableWidgetItem(rhs.Artist);
		QTableWidgetItem* item2 = new QTableWidgetItem(rhs.Title);
		QTableWidgetItem* item3 = new QTableWidgetItem(rhs.Ablue);
		QTableWidgetItem* item4 = new QTableWidgetItem(rhs.Duration);
		ui->tab_SongTable->setItem(index, 1, item1);
		ui->tab_SongTable->setItem(index, 2, item2);
		ui->tab_SongTable->setItem(index, 3, item3);
		ui->tab_SongTable->setItem(index, 4, item4);
	}*/
}

QStringList SongMenu::getSongMenu()
{
	QStringList list{};
	foreach(const UserSongMuen & rhs, songlistMenu) {
		list.push_back(rhs.name);
	}
	return list;
}


void SongMenu::on_finshedNetSongMenu(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = document.object();
			QJsonValue value = rot.value("playlist");
			if (value.isArray()) {
				UserSongMuen SongMenu;
				QJsonArray Ary = value.toArray();
				foreach(const QJsonValue & rhs, Ary) {
					if (rhs.isObject()) {
						QJsonObject obj = rhs.toObject();
						//获取歌单用户
						QJsonValue user = obj.value("creator");
						if (user.isObject()) {
							QJsonObject userobj = user.toObject();
							SongMenu.trackCount = obj.value("trackCount").toInt();
							SongMenu.name = obj.value("name").toString();
							SongMenu.id = obj.take("id").toVariant().toLongLong();
							SongMenu.coverImgUrl = obj.value("coverImgUrl").toString();
							long long t = obj.value("createTime").toVariant().toLongLong();
							QDateTime Time = QDateTime::fromMSecsSinceEpoch(t);
							SongMenu.createTime = Time.toString("yyyy-MM-dd").toUtf8();

							//歌单创建者信息
							SongMenu.userID = userobj.value("userId").toInt();
							SongMenu.nickname = userobj.value("nickname").toString();
							QString picUrl = userobj.value("avatarUrl").toString();
							QNetworkAccessManager* Netpic = new QNetworkAccessManager(this);
							QEventLoop loop;
							Netpic->get(QNetworkRequest(picUrl));
							connect(Netpic, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
								if (reply->error() == QNetworkReply::NoError) {
									SongMenu.Userpic.loadFromData(reply->readAll());
								}
								reply->deleteLater();
								});
							connect(Netpic, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
							//事件循环
							loop.exec();
						}
						songlistMenu.push_back(SongMenu);
					}
				}
			}
		}
	}
	reply->deleteLater();
}

//歌单所有歌曲  测试id = 602336734
void SongMenu::on_finsedNetAllSong(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		//我喜欢的音乐了列表ID
		if (err_t.error == QJsonParseError::NoError) {
			taglsit.clear();
			QJsonObject rot = document.object();
			M_Tag tag;
			if (!tag.ParseDetailsSong(rot, "songs")) {
				base->closeMovie();
			};
			taglsit = tag.getTag();
			curtableindex = 0;
			loadData();
		}
	}
	reply->deleteLater();
}