#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "signup.h"
#include "config.h"
#include "ui_signup.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
Signup::Signup(QDialog *parent) : QDialog(parent), ui(new Ui::Signup) {
  ui->setupUi(this);
  tip_text = ui->lab_text->text();
  this->setWindowTitle("注册");
  time = new QTimer(this);
  ui->stackedWidget->setCurrentIndex(0);
  NetSignou = new QNetworkAccessManager(this);
  Sentcaptcha = new QNetworkAccessManager(this);

  connect(time, &QTimer::timeout, this, [&]() {
    static int count = 60;
    if (count != 0)
      --count;
    ui->btn_regain->setText(QString::number(count));
    if (count == 0) {
      count = 60;
      ui->btn_regain->setEnabled(true);
      ui->btn_regain->setText("重新发送");
    }
  });
  connect(ui->line_phone, &QLineEdit::textChanged, this,
          [&] { ui->lab_text->setText(tip_text); });

  connect(ui->line_pwd, &QLineEdit::textChanged, this,
          [&] { ui->lab_text->setText(tip_text); });
  connect(NetSignou, &QNetworkAccessManager::finished, this,
          &Signup::on_finshedSingou);

  connect(Sentcaptcha, &QNetworkAccessManager::finished, this,
          &Signup::on_finshedCaptcha);
}

Signup::~Signup() { delete ui; }
QStackedWidget *Signup::getstackedwidget() { return ui->stackedWidget; }
void Signup::on_btn_back_clicked() { emit back(); }

void Signup::on_btn_back_2_clicked() { ui->stackedWidget->setCurrentIndex(0); }

//注册
void Signup::on_btn_signin_clicked() {
  //  QRegExp regx_phone("^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$");
  //  QRegExpValidator pReg{regx_phone, 0};
  //  int pos = 0;
  //  QValidator::State res = pReg.validate(phone, pos);
  //  if (!(QValidator::Acceptable == res)) {
  //    ui->line_phone->clear();
  //    ui->lab_text->setText("请输入11位有效手机号");
  //    return;
  //  }

  //  QRegExp regx_pwd{"^(?![\\d]+$)(?![a-zA-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$"};
  //  QRegExpValidator pReg_pwd{regx_pwd, 0};
  //  int pos_p = 0;
  //  QValidator::State res_p = pReg.validate(pwd, pos_p);
  //  if (!(QValidator::Acceptable == res_p)) {
  //    ui->line_pwd->clear();
  //    ui->lab_text->setText("密码8-20位,至少包含字母/数字/字符2种组合");
  //    return;
  //  }

  phone = ui->line_phone->text();
  pwd = ui->line_pwd->text();
  //发送验证码

  QString captcha_url{
      QString("http://cloud-music.pl-fe.cn/captcha/sent?phone=%1").arg(phone)};
  Sentcaptcha->get(QNetworkRequest(captcha_url));
}

//重新计时
void Signup::on_btn_regain_clicked() {
  time->start(60000);
  ui->btn_regain->setEnabled(false);
}

void Signup::on_btn_countrieslist_clicked() {
  Config cfg("../config/countrieslist.ini");
  cfg.GetAllValue("");
}

//处理注册json
void Signup::on_finshedSingou(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    const int rect =
        QMessageBox::information(this, "tip", "注册成功,是否返回登录？",
                                 QMessageBox::Yes, QMessageBox::No);
    if (rect == QMessageBox::Yes) {
      emit singin();
      this->close();
    } else {
      this->close();
    }
  }
  reply->deleteLater();
}

void Signup::on_finshedCaptcha(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      QJsonObject root = document.object();

      if (true == root.value("data").toBool()) {
        //验证码发送成功
        ui->stackedWidget->setCurrentIndex(1);
        //不可以点击，开始倒计时
        ui->btn_regain->setEnabled(false);
        time->start(60000); // 60秒

        /**
         * phone      手机号
         * pwd        密码
         * captcha    验证码
         */
        QString cap = ui->line_captcha->text();
        QString url{QString("http://cloud-music.pl-fe.cn/register/"
                            "cellphone?phone=%1&password=%2&captcha=%3")
                        .arg(phone)
                        .arg(pwd)
                        .arg(cap)};
        NetSignou->get(QNetworkRequest(url));
      } else {
        QString errmeg = root.value("message").toString();

        ui->lab_error_tip->setText(errmeg);
      }
    }
  }
  reply->deleteLater();
}