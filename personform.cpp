#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

#include "personform.h"
#include "config.h"
#include "ui_personform.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
PersonForm::PersonForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::PersonForm) {
  ui->setupUi(this);
  //设置无边框
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

  config = new Config();
  request = config->setCookies();

  NetOut = new QNetworkAccessManager(this);
  NetCheckin = new QNetworkAccessManager(this);
  
  connect(NetOut, &QNetworkAccessManager::finished, this,
          [&](QNetworkReply *reply) {
            if (reply->error() == QNetworkReply::NoError) {
              //删除配置文件
              QStringList conglist{"/cookie", "/Userinfo"};
              config->DelCongif(conglist);
              //修改checkState的值，来判断是否登录
              //config->ChangData("Pwd/loggingstatus", QVariant(0));
              //config->ChangData("Pwd/checkState", QVariant(0));
              QStringList list{"Pwd/phone", "Pwd/pwd", "Pwd/loggingstatus", "Pwd/checkState"};
              config->DelCongif(list);
              emit out();
              QMessageBox::information(this, "Tip", "退出成功",
                                       QMessageBox::Yes);
            }
            reply->deleteLater();
          });

  connect(NetCheckin, &QNetworkAccessManager::finished, this,
          &PersonForm::on_finshedCheckin);
}

PersonForm::~PersonForm() {
  delete ui;
  delete config;
  config = nullptr;
}

void PersonForm::setdynamic_nub(const QString &str) const {
  ui->lab_dynamic_nub->setText(str);
}

void PersonForm::setlab_fans_nub(const QString &str) const {
  ui->lab_fans_nub->setText(str);
}

void PersonForm::setlab_follow_nub(const QString &str) const {
  ui->lab_follow_nub->setText(str);
}

void PersonForm::setlab_level(const QString &str)const {
  ui->lab_level->setText(str);
}

void PersonForm::setlab_vip(const QString &str) const{
  ui->lab_vip->setText(str + " 到期");
}

// http://cloud-music.pl-fe.cn/daily_signin
//签到
void PersonForm::on_btn_checkin_clicked() {
  /*
   * type = 0 默认签到方式(安卓) 可获得 3 点经验
   * type = 1 web/pc签到，可获得 2 点经验
   */
  request = config->setCookies();
  QString url{"http://cloud-music.pl-fe.cn/daily_signin?type=0"};
  request->setUrl(url);
  NetCheckin->get(*request);
}

void PersonForm::on_btn_vip_clicked() {}

void PersonForm::on_btn_mark_clicked() {}

void PersonForm::on_btn_mesg_clicked() {}

void PersonForm::on_btn_bing_clicked() {}

void PersonForm::on_btn_server_clicked() {}

void PersonForm::on_btn_out_clicked() {
  QString url{"http://cloud-music.pl-fe.cn/logout"};
  request->setUrl(url);
  NetOut->get(*request);
}

//签到
void PersonForm::on_finshedCheckin(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError errt_t{};
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &errt_t);
    if (errt_t.error == QJsonParseError::NoError) {
      QJsonObject root = document.object();
      if (root.value("msg").toString() == "需要登录") {
        int rect = QMessageBox::information(this, "Error", "请先登录账号",
                                            QMessageBox::Yes, QMessageBox::No);
        if (rect == QMessageBox::Yes) {
          emit login();
          return;
        } else if (rect == QMessageBox::No) {
          return;
        }
      }
      emit CheckinOk();
    }
  }
  reply->deleteLater();
}

void PersonForm::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  this->close();
}