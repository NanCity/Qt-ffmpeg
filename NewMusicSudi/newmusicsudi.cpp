﻿#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "newmusicsudi.h"
#include "ui_newmusicsudi.h"
#include<QListView>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include<QLabel>
#include<QPushButton>
NewMusicSudi::NewMusicSudi(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::NewMusicSudi)
{
	ui->setupUi(this);

	manger = new QNetworkAccessManager(this);

	initTableView();
	NewMusicAll_Model = new QStandardItemModel(this);
	//新碟上架，全部新碟
	DiscAllGrid = new QGridLayout(ui->DiscTabwidget->widget(0));
}

NewMusicSudi::~NewMusicSudi()
{
	delete ui;
}

void NewMusicSudi::initTableView()
{
	//NewMusicAll_Model = new QStandardItemModel(ui->listView);
	////水平自适应大小
	//ui->listView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	////水平头不可见
	//ui->listView->horizontalHeader()->setVisible(false);
}

void NewMusicSudi::on_NewMusicTabwidget_tabBarClicked(int index)
{
	switch (index)
	{
	case 0:
		if (DiscAll.isEmpty()) {
			QString URL{ QString("http://localhost:3000/top/song?type=0") };
			NetNewMusic = manger->get(QNetworkRequest(URL));
			connect(NetNewMusic, &QNetworkReply::finished, this, &NewMusicSudi::on_finshedNewNewMusic);
		}
		break;
	case 1: break;
	case 2: break;
	case 3: break;
	case 4: break;
	case 5: break;
	default:
		break;
	}
}

void NewMusicSudi::on_DiscTabwidget_tabBarClicked(int index) {

}
/*
*  新歌速递
*  type: 全部:0, 华语:7, 欧美:96, 日本:8, 韩国16
*/
void NewMusicSudi::on_btn_newMusic_clicked() {
	static bool frist = true;
	if (true == frist) {
		ui->stackedWidget->setCurrentIndex(0);
		QString URL{ QString("http://localhost:3000/top/song?type=0") };
		NetNewMusic= manger->get(QNetworkRequest(URL));
		connect(NetNewMusic, &QNetworkReply::finished, this, &NewMusicSudi::on_finshedNewNewMusic);
	}
}

/*
*	新蝶上架
*	limit = 返回数量
*	area：ALL:全部,ZH:华语,EA:欧美,KR:韩国,JP:日本
*/
void NewMusicSudi::on_btn_disc_clicked()
{
	static bool disc_frist = true;
	if (true == disc_frist) {
		ui->stackedWidget->setCurrentIndex(1);

		QString URL{ QString("http://localhost:3000/album/new?area=ALL&limit=20") };
		NetDisc =manger-> get(QNetworkRequest(URL));
		connect(NetDisc, &QNetworkReply::finished, this, &NewMusicSudi::on_finshedNetDisc);
	}
}

void NewMusicSudi::on_finshedNewNewMusic()
{
	if (NetNewMusic->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(NetNewMusic->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			parseNewMusicJson(obj, "data", All);
		}
	}
	NetNewMusic->deleteLater();
}

void NewMusicSudi::parseNewMusicJson(const QJsonObject& obj, const QString& str, QList<NewMusic>& lhs)
{
	QJsonValue val = obj.value(str);
	if (val.isArray()) {
		QJsonArray ary = val.toArray();
		NewMusic NM{};
		foreach(const QJsonValue & rhs, ary) {
			NM.mp3Url = rhs.toObject().value("mp3Url").toString();

			//获取专辑封面
			if (rhs.toObject().value("album").isObject()) {
				QJsonObject albobj = rhs.toObject().value("album").toObject();
				NM.picUrl = albobj.value("picUrl").toString();

				if (albobj.value("alias").isArray()) {
					QJsonArray aliasAry = albobj.value("alias").toArray();
					NM.alias = aliasAry.at(0).toString();
				}
			}

			int t = rhs.toObject().value("duration").toInt();
			NM.duration = QString::number(static_cast<double>(t / 60), 'f', 2);

			QJsonValue art = rhs.toObject().value("artists");
			if (art.isArray()) {
				QJsonArray artAry = art.toArray();
				QJsonObject artobj = artAry.at(0).toObject();
				NM.name = artobj.value("name").toString();
				NM.id = artobj.value("id").toInt();
			}
			NM.songid = rhs.toObject().value("id").toVariant().toULongLong();
			NM.title = rhs.toObject().value("name").toString();
			lhs.append(NM);
		}
		//获取图片
		QListView* view = ui->stackedWidget->findChild<QListView*>();
		getNewMusicPic(view, lhs);
	}
}

void NewMusicSudi::on_finshedNetDisc()
{
	if (NetDisc->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument doc = QJsonDocument::fromJson(NetDisc->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = doc.object();
			parseDiscJson(obj, "albums", DiscAll);
			getDiscBtnPic(ui->DiscTabwidget->widget(0), DiscAll);
		}
	}
	NetDisc->deleteLater();
}

void NewMusicSudi::parseDiscJson(const QJsonObject& obj, const QString& str, QList<NewDisc>& _Disc)
{
	QJsonValue val = obj.value(str);
	NewDisc disc{};
	if (val.isArray()) {
		QJsonArray Ary = val.toArray();
		foreach(const QJsonValue & rhs, Ary) {
			disc.picUrl = rhs.toObject().value("picUrl").toString();
			disc.Title = rhs.toObject().value("name").toString();
			disc.discID = rhs.toObject().value("id").toVariant().toLongLong();
			if (rhs.toObject().value("artists").isObject()) {
				QJsonObject Arobj = rhs.toObject().value("artists").toObject();
				disc.name = Arobj.value("name").toString();
				disc.artid = Arobj.value("id").toVariant().toLongLong();
			}
			_Disc.push_back(disc);
		}
	}
	getDiscBtnPic(ui->stackedWidget->currentWidget(), _Disc);
}

void NewMusicSudi::loadDiscAllData(QWidget* wid, QList<NewDisc>& rhs, const QPixmap& pix)
{
	QPushButton* btn = new QPushButton("TT", ui->DiscTabwidget->widget(0));
	btn->setStyleSheet("background-color:red");
	btn->resize(80, 80);
	btn->setIconSize(QSize(80, 80));
	btn->setIcon(pix);
	DiscAllGrid->addWidget(btn);
	ui->DiscTabwidget->widget(0)->setLayout(DiscAllGrid);

}

//获取新歌速递的专辑图片
void NewMusicSudi::getNewMusicPic(QListView* wid, QList<NewMusic>& rhs)
{
	//QEventLoop loop;
	//connect(&pic, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

	foreach(const NewMusic & lhs, rhs) {
		pic = manger->get(QNetworkRequest(lhs.picUrl));
		//loop.exec();
	}
	connect(pic, &QNetworkReply::finished, this, [&]() {
		if (pic->error() == QNetworkReply::NoError) {
			QPixmap pix;
			pix.loadFromData(pic->readAll());
			QStandardItem* item = new QStandardItem();
			item->setData(rhs.at(1).name);
			NewMusicAll_Model->appendRow(item);
			wid->setModel(NewMusicAll_Model);
			qDebug() << "进入getNewMusicPic()\n";
		}
		pic->deleteLater();
		}, Qt::QueuedConnection);
	
	//loadNewMusicAllData(wid, rhs, pixlist);
}

void NewMusicSudi::loadNewMusicAllData(QListView* wid, QList<NewMusic>& rhs, const QList<QPixmap>& pix)
{

	//QStandardItemModel* model = new QStandardItemModel(ui->listView);
	//QStandardItem* item = new QStandardItem();
	////pixlist.push_back(pix);
	//ui->listView->setIndexWidget(item->index(), addListViewData(1, rhs, pix));
	qDebug() << "新歌速递";
}

//插入widget到listView
QWidget* NewMusicSudi::addListViewData(const int index, QList<NewMusic>& rhs, const QPixmap& pix)
{
	QWidget* wid = new QWidget(this);
	QGridLayout* grid = new QGridLayout();
	QHBoxLayout* hbox = new QHBoxLayout();
	QLabel* pic = new QLabel(wid);
	pic->setScaledContents(true);
	pic->setPixmap(pix);
	pic->setMinimumSize(60, 60);
	pic->setMaximumSize(65, 65);
	hbox->addWidget(pic);

	QLabel* title = new QLabel(rhs.at(index).title, wid);
	//弹簧
	QSpacerItem* item1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hbox->addWidget(title);
	hbox->addItem(item1);

	QPushButton* art = new QPushButton(rhs.at(index).name, wid);
	QString objname{ QString("Art_%1").arg(index) };
	art->setObjectName(objname);
	hbox->addWidget(art);
	NewMusicBtnMap.insert(objname, rhs.at(index));

	QPushButton* alb = new QPushButton(rhs.at(index).title, wid);
	QString albName{ QString("Alb_%1").arg(index) };
	alb->setObjectName(albName);
	NewMusicBtnMap.insert(albName, rhs.at(index));
	QSpacerItem* item2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hbox->addItem(item2);
	hbox->addWidget(alb);

	QLabel* time = new QLabel(rhs.at(index).duration, wid);
	QSpacerItem* item3 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hbox->addItem(item3);
	hbox->addWidget(time);

	grid->addLayout(hbox, 0, 0, 1, 1);
	wid->setLayout(grid);

	wid->setMaximumHeight(870);
	return wid;
}

void NewMusicSudi::getDiscBtnPic(QWidget* wid, QList<NewDisc>& data)
{
	QEventLoop loop;
	foreach(const NewDisc & rhs, data) {
		Netgetpic = manger->get(QNetworkRequest(rhs.picUrl));
		connect(Netgetpic, &QNetworkReply::finished, this, [&]() {
			if (Netgetpic->error() == QNetworkReply::NoError) {
				QPixmap pix;
				pix.loadFromData(Netgetpic->readAll());
				loadDiscAllData(wid, data, pix);
				qDebug() << "on_finshedNetgetpic*********";
			}
			Netgetpic->deleteLater();
			}, Qt::QueuedConnection);
		loop.exec();
	}
}
