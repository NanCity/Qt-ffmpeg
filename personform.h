#ifndef PERSONFORM_H
#define PERSONFORM_H

#include <QWidget>
class Config;
namespace Ui {
class PersonForm;
}
class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;

class PersonForm : public QWidget {
  Q_OBJECT

public:
  explicit PersonForm(QWidget *parent = nullptr);
  virtual ~PersonForm();
  void setlab_level(const QString &str)const;
  void setlab_vip(const QString &str)const;
  void setdynamic_nub(const QString &str)const;
  void setlab_fans_nub(const QString &str)const;
  void setlab_follow_nub(const QString &str)const;
protected slots:
  //签到
  void on_btn_checkin_clicked();
  // vip
  void on_btn_vip_clicked();
  //商场
  void on_btn_mark_clicked();
  //个人信息设置
  void on_btn_mesg_clicked();
  //绑定社交账号
  void on_btn_bing_clicked();

  //我的客服
  void on_btn_server_clicked();
  //退出
  void on_btn_out_clicked();

  //网络请求部分
  void on_finshedCheckin();
  void leaveEvent(QEvent *event);

  //退出登录
  void on_logout();
signals:
  void out();
  void CheckinOk();
  void login();

private:
  Ui::PersonForm *ui;
  Config *config;
  QNetworkRequest *request;
  QNetworkAccessManager* manger;
  QNetworkReply* NetOut{};
  QNetworkReply *NetCheckin{};
};

#endif // PERSONFORM_H
