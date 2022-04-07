#include "singerdetails.h"
#include "ui_singerdetails.h"
#include "tag.h"
#include "base.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QGridLayout>


SingerDetails::SingerDetails(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::SingerDetails)
{
	ui->setupUi(this);
	base = new Base(ui->top50_list);
	ui->top50_list->horizontalHeader()->hide();
	ui->top50_list->setColumnCount(3);
	//设置第一列表头自适应widget宽高
	ui->top50_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//图片自适应
	ui->lab_Singerpic->setScaledContents(true);
	ui->lab_top50pic->setScaledContents(true);
	NetSingerDet = new QNetworkAccessManager(this);
	NetTop50 = new QNetworkAccessManager(this);
	NetMV = new QNetworkAccessManager(this);
	NetGetPic = new QNetworkAccessManager(this);
	connect(NetSingerDet, &QNetworkAccessManager::finished, this, &SingerDetails::on_finsedNetSingerDet);
	connect(NetTop50, &QNetworkAccessManager::finished, this, &SingerDetails::on_finshedNetTop50);
	connect(NetMV, &QNetworkAccessManager::finished, this, &SingerDetails::on_finshedNetMV);
	connect(NetGetPic, &QNetworkAccessManager::finished, this, &SingerDetails::on_finshedGetPic);

	//菜单按钮
	connect(base->Actcomment, &QAction::triggered, this, [&]() {});
	connect(base->Actplay, &QAction::triggered, this, [&]() {
		int current = ui->top50_list->currentRow();
		emit play(this, current);
		});

	connect(base->Actnextplay, &QAction::triggered, this, [&]() {
		int current = ui->top50_list->currentRow();
		emit Nextplay(this, current, songid.at(current));
		});

	connect(base->Actcollect, &QAction::triggered, this, [&]() {});
	connect(base->Actdownload, &QAction::triggered, this, [&]() {});


}

SingerDetails::~SingerDetails()
{
	delete ui;
}

void SingerDetails::setmesg(Singer singer)
{
	ui->lan_Artist->setText(singer.getname());
	ui->lab_songNum->setText(QString::number(singer.getmusicSize()) + "\t");
	ui->lab_MVNum->setText(QString::number(singer.getmvSize()) + "\t");
	ui->lab_AlbumNum->setText(QString::number(singer.getalbumSize()) + "\t");
	ui->lab_briefDesc->setText(singer.getname() + "的简介:\n  " + singer.getbriefDesc());
	emit loadok();
}

void SingerDetails::setID(const int id)
{
	singerID = id;
	QString Url{ QString("http://localhost:3000/artist/detail?id=%1").arg(id) };
	NetSingerDet->get(QNetworkRequest(Url));

	//歌手top50首
	QString topUrl{ QString("http://localhost:3000/artist/top/song?id=%1").arg(id) };
	NetTop50->get(QNetworkRequest(topUrl));
}


//播放全部
void SingerDetails::on_btn_playAll_clicked()
{
	emit playAll(this);
}

void SingerDetails::on_finsedNetSingerDet(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		Singer singer{};
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = document.object();
			QJsonValue data = rot.value("data");
			if (data.isObject()) {
				QJsonObject dataRot = data.toObject();
				QJsonValue art = dataRot.value("artist");

				if (art.isObject()) {
					singer.setid(art.toObject().value("id").toInt());
					singer.setname(art.toObject().value("name").toString());
					//歌手的描述
					singer.setbriefDesc(art.toObject().value("briefDesc").toString());
					//专辑数量
					singer.setalbumSize(art.toObject().value("albumSize").toInt());
					//音乐数量
					singer.setmusicSize(art.toObject().value("musicSize").toInt());
					//MV数量
					singer.setMVSize(art.toObject().value("mvSize").toInt());
					QString artpicUrl = art.toObject().value("cover").toString();

					QNetworkAccessManager* Netpci = new QNetworkAccessManager(this);
					//QEventLoop loop;
					Netpci->get(QNetworkRequest(artpicUrl));
					//connect(Netpci, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
					connect(Netpci, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
						if (reply->error() == QNetworkReply::NoError) {
							QPixmap pix;
							pix.loadFromData(reply->readAll());
							ui->lab_Singerpic->setPixmap(pix);
							//singer.setpixmap(pix);
						}
						reply->deleteLater();
						});
					//loop.exec();
					setmesg(singer);
				}
			}
		}

	}

	reply->deleteLater();
}

void SingerDetails::on_finshedNetTop50(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = document.object();
			tag.ParseDetailsSong(rot, "songs");
			taglist = tag.getTag();
			int index = 0;
		}
		loadData();
	}

	reply->deleteLater();
}




void SingerDetails::on_tabWidget_tabBarClicked(int index)
{
	switch (index) {
	case 1:
	{
		/*
		* id = 歌手ID
		*/
		QString MVURL{ QString("http://localhost:3000/artist/mv?id=%1").arg(singerID) };
		NetMV->get(QNetworkRequest(MVURL));
	}
	break;
	case 3:
		//相似歌手
		break;
	default:
		break;
	}
}


//获取歌手MV
void SingerDetails::on_finshedNetMV(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject obj = document.object();
			QJsonValue val = obj.value("mvs");
			if (val.isArray()) {
				QJsonArray ary = val.toArray();
				MV _mv{};
				mv.clear();
				mvpic.clear();
				foreach(const QJsonValue & rhs, ary) {
					if (rhs.isObject()) {
						QJsonObject obj = rhs.toObject();
						_mv.imgurl = obj.value("imgurl").toString();

						_mv.id = obj.value("id").toInt();
						_mv.artistName = obj.value("artistName").toString();
						int time = obj.value("duration").toInt();
						_mv.duration = QString(QString::number(static_cast<double>(time / 60), 'f', 2));
						_mv.playCount = obj.value("playCount").toVariant().toULongLong();
						_mv.publishTime = obj.value("publishTime").toString();
						mv.push_back(_mv);
						NetGetPic->get(QNetworkRequest(_mv.imgurl));
					}
				}
				//会抛异常
				//addMVtoTabwidget();
			}
		}
	}
	reply->deleteLater();
}


void SingerDetails::on_finshedGetPic(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QPixmap pix{};
		pix.loadFromData(reply->readAll());
		mvpic.push_back(pix);
	}
	qDebug() << "on_finshedGetPic";
	if (mvpic.length() == mv.length()) {
		addMVtoTabwidget();
	}
	reply->deleteLater();
}


void SingerDetails::addMVtoTabwidget()
{
	//mvbtn.clear();
	QString objname{};
	int len = mv.length();
	int row = 0, colum = 0;
	QGridLayout* grid = new QGridLayout(ui->tab_2);
	for (int i = 0; i != len; ++i)
	{
		QPushButton* _btn = new QPushButton(ui->tab_2);
		objname = QString("btn_%1").arg(i);
		_btn->setObjectName(objname);
		_btn->setMinimumSize(180, 90);
		_btn->setMaximumSize(240, 140);
		_btn->setStyleSheet("background-color:red");

		if (i > 0 && i % 5 == 0) {
			//又从第0列开始
			colum = 0;
			++row;
		}
		grid->addWidget(_btn, row, colum);
		++colum;
		Mv_Btn_ID.insert(objname, mv.at(i).id);
		connect(_btn, &QPushButton::clicked, this, &SingerDetails::on_mvBtnClicked);
		//mvbtn.push_back(_btn);
		//设置封面
		_btn->setIconSize(_btn->size());
		_btn->setIcon(mvpic.at(i));
		ui->tab_2->setLayout(grid);
	}

}



void SingerDetails::on_mvBtnClicked()
{
	auto obj = sender();
	auto it = Mv_Btn_ID.find(obj->objectName());
	qDebug() << "objname = " << it.key() << " id = " << it.value();
}

void SingerDetails::loadData()
{
	Config config("../config/MyLikeMusicId.ini");
	QList<int>* likeID = &config.GetLikeMusicId();
	int len = taglist->length();
	int i = 0;
	while (i != len)
	{
		ui->top50_list->insertRow(i);
		if (likeID->at(i) == taglist->at(i).Songid) {
			ui->top50_list->setCellWidget(i, 0, base->setItemWidget(1));
		}
		else
		{
			ui->top50_list->setCellWidget(i, 0, base->setItemWidget(0));
		}

		QTableWidgetItem* item = new QTableWidgetItem(taglist->at(i).Title);
		QTableWidgetItem* item2 = new QTableWidgetItem(taglist->at(i).Duration);
		ui->top50_list->setItem(i, 1, item);
		ui->top50_list->setItem(i, 2, item2);
		++i;
	}
}