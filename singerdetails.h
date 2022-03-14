#ifndef SINGERDETAILS_H
#define SINGERDETAILS_H

#include <QWidget>
#include "tag.h"
#include <QQueue>
class Base;
class Singer;
class QNetworkAccessManager;
class QNetworkReply;

class MV {
public:
	int id{};
	QString duration{};
	//播放次数
	size_t playCount{};
	QString artistName{};
	QString imgurl{};
	QString publishTime{};
};

namespace Ui {
	class SingerDetails;
}

class SingerDetails : public QWidget
{
	Q_OBJECT

public:
	explicit SingerDetails(QWidget* parent = nullptr);
	~SingerDetails();
	void setmesg(Singer singer);
	void setID(const int id);
	void loadData();
	void addMVtoTabwidget();
	void getMvPic();
	QStringList GetPlayList() { return songid; }
	//QList<unsigned int> GetPlaylistID() { return ID; }
protected slots:
	void on_btn_playAll_clicked();
	void on_finsedNetSingerDet(QNetworkReply* reply);
	void on_finshedNetTop50(QNetworkReply* reply);
	void on_finshedNetMV(QNetworkReply* reply);
	void on_finshedGetPic(QNetworkReply* reply);
	//选项卡被点击
	void on_tabWidget_tabBarClicked(int index);
	void on_mvBtnClicked();
signals:
	void playAll(SingerDetails*);
	void play(SingerDetails*, const int index);
	void Nextplay(SingerDetails*, const int index, const QString ID);
	void loadok();
private:
	Ui::SingerDetails* ui;
	int singerID{};
	Base* base;
	M_Tag tag;
	QList<Temptag>* taglist;

	QList<MV>	mv{};
	QList<QPixmap> mvpic{};
	//QList<QPushButton*>mvbtn;
	QMap<QString, int> Mv_Btn_ID{};
	QQueue<QNetworkReply*>queueReply;
	QStringList songid{};
	//QQueue<QNetworkReply*>MV_PIC_reply;
	QNetworkAccessManager* NetSingerDet;
	QNetworkAccessManager* NetTop50;
	QNetworkAccessManager* NetMV;
	QNetworkAccessManager* NetGetPic;
};

#endif // SINGERDETAILS_H
