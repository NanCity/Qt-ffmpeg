﻿#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QJsonObject>
//封装了JSONd支持的数据类型
#include <QJsonValue>

namespace Ui {
	class Login;
}

#define Error(string)                                                          \
  qDebug() << __FILE__ << ' ' << __LINE__ << ' ' << string << '\n';
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QListWidgetItem;
class QListWidget;
class QRegExp;
class Config;
class QPixmap;
class Signup;
class QTimer;
// class ParseLoadJson;

// class Personal_information {
// public:
//  /*用户ID*/
//  int ID;
//  /*用户名*/
//  QString UserName;
//  /*背景图片*/
//  QString BackgroundUrl;
//  /*用户头像*/
//  QString AvatarUrl;
//  /*用户粉丝*/
//  int Fans;
//  /*关注人数*/
//  int Follows;
//  /*歌单*/
//  int playlistCount;
//  //保存cookie
//  QString cookie;
//
// public:
//  Personal_information();
//  ~Personal_information();
//};

class Login : public QDialog {
	Q_OBJECT

public:
	explicit Login(QWidget* parent = nullptr);
	virtual ~Login();
	bool ParseJson(QJsonObject& value);
	void AutoLogin();
	//扫码确认登陆后,获取用户信息
	void GetUserInfo(const int& UserId);
	void Grade();
	void Vip();
	QPixmap getPix() { return pix; }
private slots:
	//时间过滤器
	bool eventFilter(QObject* obj, QEvent* event);

	void on_btn_signin_clicked();
	void on_btn_signup_clicked();
	void on_btn_Qrcode_clicked();
	void countrieslist_itemClicked(QListWidgetItem* item);
	//获取国家编码列表
	void on_btn_countrieslist_clicked();
	//返回登录界面
	void on_btn_backlogin_clicked();
	//扫码登录
	void on_btn_QRC_clicked();

	void on_replyFinished();
	void on_FinshedPic();
	bool checkInput(QString str, const QRegExp regx, const int n);
	void on_finshedGrade();
	void on_finshedVip();
	void on_finshedUserMsg();
	void on_finshedCountriesList();
	void on_finshedNetUserInfo();
	//处理二维码登录
	void on_finshedQRC_key();
	void on_finshedQRC_create();
	void on_finshedQRC_check();

	void on_pushButton_clicked();
	//二维码过期
	void on_QRCexpired();
signals:
	void LoginSucces();

private:
	Ui::Login* ui;
	//用于检测扫码状态
	QTimer* time;
	//每隔2秒确认一次是否扫描
	QTimer* request_time;
	Signup* signup;
	int level;
	QString phone{};
	QString pwd{};
	QPixmap pix;
	Config* config;
	QString QRC_KEY;
	QListWidget* countrieslist;
	QNetworkRequest* NetRequest;
	QNetworkAccessManager* manger;
	//发送网络请求和接受应答
	QNetworkReply* NetManager{};
	QNetworkReply* NetUserPic{};
	QNetworkReply* Netgrade{};
	QNetworkReply* NetVip{};
	QNetworkReply* NetUserMsg{};
	QNetworkReply* NetCountries{};
	QNetworkReply* NetUserInfo{};
	//二维码登录
	QNetworkReply* NetQRC_key{};
	QNetworkReply* NetQRC_create{};
	QNetworkReply* NetQRC_check{};
};

#endif // LOGIN_H
