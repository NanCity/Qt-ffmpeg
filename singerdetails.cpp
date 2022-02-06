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
SingerDetails::SingerDetails(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::SingerDetails)
{
	ui->setupUi(this);
	base = new Base(ui->top50_list);
	ui->top50_list->horizontalHeader()->hide();
	ui->top50_list->setColumnCount(3);
	ui->top50_list->horizontalHeader()->setSectionResizeMode(
		0, QHeaderView::Fixed);

	//图片自适应
	ui->lab_Singerpic->setScaledContents(true);
	ui->lab_top50pic->setScaledContents(true);
	NetSingerDet = new QNetworkAccessManager(this);
	NetTop50 = new QNetworkAccessManager(this);
	connect(NetSingerDet, &QNetworkAccessManager::finished, this, &SingerDetails::finsedNetSingerDet);
	connect(NetTop50, &QNetworkAccessManager::finished, this, &SingerDetails::finshedNetTop50);


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
	ui->lab_Singerpic->setPixmap(singer.getPixmap());
	ui->lan_Artist->setText(singer.getname());
	ui->lab_songNum->setText(QString::number(singer.getmusicSize()) + "\t");
	ui->lab_MVNum->setText(QString::number(singer.getmvSize()) + "\t");
	ui->lab_AlbumNum->setText(QString::number(singer.getalbumSize()) + "\t");
	emit loadok();
}

void SingerDetails::setID(const int id)
{
	singerID = id;
	QString Url{ QString("http://cloud-music.pl-fe.cn/artist/detail?id=%1").arg(id) };
	NetSingerDet->get(QNetworkRequest(Url));

	//歌手top50首
	QString topUrl{ QString("http://cloud-music.pl-fe.cn/artist/top/song?id=%1").arg(id) };
	NetTop50->get(QNetworkRequest(topUrl));
}

//播放全部
void SingerDetails::on_btn_playAll_clicked()
{
	emit playAll(this);
}

void SingerDetails::finsedNetSingerDet(QNetworkReply* reply)
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
					QEventLoop loop;
					Netpci->get(QNetworkRequest(artpicUrl));
					connect(Netpci, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
					connect(Netpci, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {
						if (reply->error() == QNetworkReply::NoError) {
							QByteArray bty{ reply->readAll() };
							singer.setpixmap(bty);
						}
						reply->deleteLater();
						});
					loop.exec();
					setmesg(singer);
				}
			}
		}

	}

	reply->deleteLater();
}

void SingerDetails::finshedNetTop50(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = document.object();
			M_Tag tag;

			tag.ParseDetailsSong(rot, "songs");
			QList<Temptag> taglist = tag.getTag();
			//ID.clear();
			songid.clear();
			int index = 0;
			foreach(const Temptag & rhs, taglist) {
				ui->top50_list->insertRow(index);
				ui->top50_list->setCellWidget(index, 0, base->setItemWidget());
				QTableWidgetItem* item = new QTableWidgetItem(rhs.Title);
				QTableWidgetItem* item2 = new QTableWidgetItem(rhs.Duration);
				ui->top50_list->setItem(index, 1, item);
				ui->top50_list->setItem(index, 2, item2);
				//保存ID
				/*QString Url{QString("https://music.163.com/song/media/outer/url?id=%1").arg(rhs.Songid)};*/
				songid.push_back(QString::number(rhs.Songid));
				//ID.push_back(rhs.Songid);

				++index;
			}
		}
	}
	reply->deleteLater();
}
