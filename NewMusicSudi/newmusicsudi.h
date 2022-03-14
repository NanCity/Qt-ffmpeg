#ifndef NEWMUSICSUDI_H
#define NEWMUSICSUDI_H

#include <QWidget>
#include <QNetworkAccessManager>
//class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;
class QGridLayout;
class QTableView;
class QListView;
class QStandardItemModel;
namespace Ui {
	class NewMusicSudi;
}

//新碟
struct NewDisc {
	//作家ID
	size_t artid{};
	//新碟ID
	size_t discID{};
	//作家
	QString name{};
	//标题
	QString Title{};
	//封面
	QString picUrl{};
};

//对于自定义数据类型，如果要使用QVariant，就必须使用Q_DECLARE_METATYPE注册。
 Q_DECLARE_METATYPE(NewDisc)

//新歌速递
struct NewMusic {
	//作家ID
	int id{};
	//时常
	QString duration{};
	//歌曲ID
	size_t songid{};
	//歌曲播放地址
	QString mp3Url{};
	//歌名后面一小窜灰色字体
	QString alias{};
	//作家
	QString name{};
	//歌曲名称
	QString title{};
	//封面
	QString picUrl{};
};

class NewMusicSudi : public QWidget
{
	Q_OBJECT

public:
	explicit NewMusicSudi(QWidget* parent = nullptr);
	virtual ~NewMusicSudi();
	void initTableView();

	void parseNewMusicJson(const QJsonObject& obj, const QString& str, QList<NewMusic>&);
	void parseDiscJson(const QJsonObject& obj, const QString& str, QList<NewDisc>&);

	void loadDiscAllData(QWidget* wid, QList<NewDisc>& rhs, const QPixmap& pix);
	void loadNewMusicAllData(QListView* wid, QList<NewMusic>& rhs, const QList<QPixmap>& pix);

	void getDiscBtnPic(QWidget* wid, QList<NewDisc>& rhs);
	void getNewMusicPic(QListView* wid, QList<NewMusic>& rhs);

	QWidget* addListViewData(const int index, QList<NewMusic>& rhs, const QPixmap& pix);
protected slots:
	//新歌速递tabwidget bar被点击
	void on_NewMusicTabwidget_tabBarClicked(int index);
	//新碟tabwidget bar被点击
	void on_DiscTabwidget_tabBarClicked(int index);
	void on_btn_newMusic_clicked();
	void on_btn_disc_clicked();
	void on_finshedNewNewMusic(QNetworkReply*);
	void on_finshedNetDisc(QNetworkReply*);
private:
	Ui::NewMusicSudi* ui;
	//新歌速递部分
	QStandardItemModel* NewMusicAll_Model;
	QList<NewMusic> All{};
	QMap<QString, NewMusic> NewMusicBtnMap{};

	//新碟
	QList<NewDisc> DiscAll{};
	QGridLayout* DiscAllGrid;
	QNetworkAccessManager *manger;
	QNetworkAccessManager NetNewMusic{};

	QNetworkAccessManager* NetDisc;
	QNetworkAccessManager* Netgetpic;

	QNetworkAccessManager pic{};
};

#endif // NEWMUSICSUDI_H
