#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "login.h"
#include "config.h"
#include "signup.h"
#include "ui_login.h"
#include <QDateTime>
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTimer>
Login::Login(QWidget *parent) : QDialog(parent), ui(new Ui::Login) {
  ui->setupUi(this);
  ui->stackedWidget->setCurrentIndex(0);
  this->setWindowTitle("登录");
  config = new Config();
  signup = new Signup(this);
  time = new QTimer(this);
  request_time = new QTimer(this);
  countrieslist = new QListWidget(this);
  countrieslist->installEventFilter(this);
  countrieslist->hide();
  signup->hide();
  //处于输入状态的时候，是正常显示字符。 输入完毕之后,使用Password形式隐藏字符
  ui->line_word->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  NetManager = new QNetworkAccessManager{this};
  NetUserPic = new QNetworkAccessManager{this};
  NetRequest = new QNetworkRequest{};
  Netgrade = new QNetworkAccessManager(this);
  NetVip = new QNetworkAccessManager(this);
  NetUserMsg = new QNetworkAccessManager(this);
  NetCountries = new QNetworkAccessManager(this);
  connect(signup, &Signup::back, this, [&]() {
    signup->hide();
    this->show();
  });

  connect(countrieslist, &QListWidget::itemClicked, this,
          &Login::countrieslist_itemClicked);

  //请求结束以后调用该槽
  connect(NetManager, &QNetworkAccessManager::finished, this,
          &Login::on_replyFinished);
  connect(NetUserPic, &QNetworkAccessManager::finished, this,
          &Login::on_FinshedPic);

  connect(Netgrade, &QNetworkAccessManager::finished, this,
          &Login::on_finshedGrade);

  connect(NetVip, &QNetworkAccessManager::finished, this,
          &Login::on_finshedVip);

  connect(signup, &Signup::singin, this, &Login::show);

  connect(time, &QTimer::timeout, this, &Login::on_QRCexpired);

  //二维码登录
  NetQRC_key = new QNetworkAccessManager(this);
  NetQRC_create = new QNetworkAccessManager(this);
  NetQRC_check = new QNetworkAccessManager(this);

  connect(NetQRC_key, &QNetworkAccessManager::finished, this,
          &Login::on_finshedQRC_key);
  connect(NetQRC_create, &QNetworkAccessManager::finished, this,
          &Login::on_finshedQRC_create);
  connect(NetQRC_check, &QNetworkAccessManager::finished, this,
          &Login::on_finshedQRC_check);

  connect(NetUserMsg, &QNetworkAccessManager::finished, this,
          &Login::on_finshedUserMsg);

  connect(NetCountries, &QNetworkAccessManager::finished, this,
          &Login::on_finshedCountriesList);
  AutoLogin();
}

Login::~Login() {
  delete ui;
  delete config;
  config = nullptr;
  delete NetRequest;
  NetRequest = nullptr;
}

bool Login::eventFilter(QObject *obj, QEvent *event) {
  if (obj == countrieslist) {
    if (event->type() == QEvent::Leave) {
      countrieslist->close();
    }
  }
  // return Login::eventFilter(obj, event);
  return false;
}

//解析json文件
bool Login::ParseJson(QJsonObject &rootobj) {
  //获取用户信息
  QJsonValue profileobj = rootobj.value("profile");
  //判断这个profile对象是否是object
  if (profileobj.isObject()) {
    //将其转换成object
    auto profile = profileobj.toObject();
    //将用户信息写入配置文件
    QString key{"Userinfo"};
    config->SetBeginGroup(key);
    config->SetValue("userId", QVariant(profile.value("userId").toInt()));
    config->SetValue("nickname",
                     QVariant(profile.value("nickname").toString()));
    config->SetValue("avatarUrl",
                     QVariant(profile.value("avatarUrl").toString()));
    config->SetValue("backgroundUrl",
                     QVariant(profile.value("backgroundUrl").toString()));
    config->SetValue("followeds", QVariant(profile.value("followeds").toInt()));
    config->SetValue("follows", QVariant(profile.value("follows").toInt()));
    config->SetValue("playlistCount",
                     QVariant(profile.value("playlistCount").toInt()));

    config->endGroup();

    NetUserPic->get(QNetworkRequest(profile.value("avatarUrl").toString()));
    return true;
  }
  return false;
}

void Login::AutoLogin() {
  //自动登录，默认打勾
  ui->checkBox->setCheckState(Qt::Checked);
  if (config->GetValue("/Pwd/checkState") == "1") {
    phone = config->GetValue("/Pwd/phone");
    pwd = config->GetValue("/Pwd/pwd");
    ui->line_phoneID->setText(phone);
    ui->line_word->setText(pwd);
    if (phone.isEmpty() && pwd.isEmpty()) {
      return;
    }

    NetManager->get(QNetworkRequest(
        QString(
            "http://cloud-music.pl-fe.cn/login/cellphone?phone=%1&password=%2")
            .arg(phone)
            .arg(pwd)));
    emit LoginSucces();
    system("cls");
    fprintf(stdout, "***自动登录完成\n***");
  }
}

void Login::Grade() {
  QString url{"http://cloud-music.pl-fe.cn/user/level"};
  QNetworkRequest *request = config->setCookies();
  request->setUrl(url);
  Netgrade->get(*request);
}

void Login::Vip() {
  QString url{"http://cloud-music.pl-fe.cn/vip/info"};
  QNetworkRequest *requests = config->setCookies();
  requests->setUrl(url);
  NetVip->get(*requests);
}

//登陆
void Login::on_btn_signin_clicked() {
  //验证手机号码 1,2用于区分是密码错误还是手机号码错误
  static QRegExp regx_phone(
      "^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$");
  if (false == checkInput(ui->line_phoneID->text(), regx_phone, 1)) {
    return;
  }
  //验证密码 8-16位且必须为数字、大小写字母或符号中至少2种
  static QRegExp regx_pwd{
      "^(?![\\d]+$)(?![a-zA-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$"};
  if (false == checkInput(ui->line_word->text(), regx_pwd, 2)) {
    return;
  }

  QString phone = ui->line_phoneID->text();
  QString psw = ui->line_word->text();
  //判断一下网络状态， 如果为NotAccessible重新设置一下
  if (NetManager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
    NetManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }

  QString countrycode =
      ui->btn_countrieslist->text().split(QRegExp("[+]")).at(1);
  //以get方式发送一个网络请求
  /*
   * phone 手机号
   * countrycode 国家码
   * password 密码
   */
  NetManager->get(
      QNetworkRequest(QString("http://cloud-music.pl-fe.cn/login/"
                              "cellphone?phone=%1&countrycode=%2&password=%3")
                          .arg(phone)
                          .arg(countrycode)
                          .arg(psw)));
}
//注册
void Login::on_btn_signup_clicked() {
  this->hide();
  signup->show();
}

void Login::countrieslist_itemClicked(QListWidgetItem *item) {
  //截取右边5个字符
  QStringList str = item->data(0).toString().split(QRegExp("[+]"));
  ui->btn_countrieslist->setText("+" + str.at(1));
}

void Login::on_btn_countrieslist_clicked() {
  Config cfg("../ config / countrieslist.ini");
  int index = cfg.SetBeginReadAry("countrieslist");
  if (index > 0) {
    for (int i = 0; i != index; ++i) {
      config->SetAryIndex(i);
      QString zh = cfg.GetValue("en");
      QString code = cfg.GetValue("code");
      qDebug() << zh << ' ' << code << '\n';
      countrieslist->addItem(
          new QListWidgetItem(QString("%1\t\t+%2").arg(zh).arg(code)));
    }
    cfg.endArray();

    countrieslist->resize(260, 220);
    countrieslist->move(35, 220);
    countrieslist->show();
  } else {
    //说明 : 调用此接口, 可获取国家编码列表
    QString url{"http://cloud-music.pl-fe.cn/countries/code/list"};
    NetCountries->get(QNetworkRequest(url));
  }
}

void Login::on_btn_backlogin_clicked() {
  ui->stackedWidget->setCurrentIndex(0);
}

/*
 * 二维码登录
 * key 请求对象
 * time  时间戳
 */
void Login::on_btn_QRC_clicked() {
  //获取当前时间
  QDateTime _time = QDateTime::currentDateTime();
  //将当前时间转为时间戳
  uint timeT = _time.toTime_t();
  QString Qrc_Key{
      QString("http://cloud-music.pl-fe.cn/login/qr/key?time=%1").arg(timeT)};
  NetQRC_key->get(QNetworkRequest(Qrc_Key));
  ui->stackedWidget->setCurrentIndex(1);
}

void Login::on_replyFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QString all = reply->readAll();
    QByteArray byte_array{};
    byte_array.append(all);
    QJsonParseError err_rpt{};
    // QJsonDocument封装了完整的json文档
    QJsonDocument document = QJsonDocument::fromJson(byte_array, &err_rpt);
    if (err_rpt.error == QJsonParseError::NoError) {
      QJsonObject rootobj = document.object();
      QString mes = rootobj.value("msg").toString();
      if (mes == "密码错误") {
        fprintf(stdout, mes.toStdString().data());
        ui->lab_error->setText(mes);
        return;
      }
      //获取等级
      Grade();
      //获取vip信息
      Vip();
      //获取登录信息
      for (auto it = rootobj.begin(); it != rootobj.end(); ++it) {
        if (ParseJson(rootobj)) {
          //获取请求头
          if (reply->hasRawHeader("Set-Cookie")) {
            QByteArray cookie = reply->rawHeader("Set-Cookie");
            //将cookie写入配置文件
            config->SetBeginGroup("cookie");
            config->SetValue("cookie", QVariant(cookie));
            config->endGroup();
          }
        }
      }

      //记录登录密码
      config->SetBeginGroup("Pwd");
      if (ui->checkBox->checkState() == Qt::Checked) {
        phone = ui->line_phoneID->text();
        pwd = ui->line_word->text();
        config->SetValue("phone", QVariant(phone));
        config->SetValue("pwd", QVariant(pwd));
        config->SetValue("checkState", QVariant(1));
        config->SetValue("loggingstatus", QVariant(1));
      } else {
        config->SetValue("checkState", QVariant(0));
      }
      config->endGroup();

      this->close();

      emit LoginSucces();
    } else {
      Error(tr("Json format error"));
      return;
    }
  }
  reply->deleteLater();
}

void Login::on_FinshedPic(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    pix.loadFromData(reply->readAll());
    pix.save("../Userpix/user.png");
  }
  reply->deleteLater();
}

bool Login::checkInput(QString str, const QRegExp regx, const int n) {
  QRegExpValidator pReg{regx, 0};
  int pos = 0;
  QValidator::State res = pReg.validate(str, pos);
  switch (n) {
  case 1:
    //输入的格式不对
    if (!(QValidator::Acceptable == res)) {
      ui->lab_error->setText("请输入11位手机号码");
      ui->line_phoneID->clear();
      ui->line_word->clear();
      return false;
    }
    break;
  case 2:
    if (!(QValidator::Acceptable == res)) {
      ui->lab_error->setText("8-16位数字、大小写字母或符号中至少2种");
      ui->line_word->clear();
      return false;
    }
    break;
  }
  return true;
}

void Login::on_finshedGrade(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError errt_t{};
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &errt_t);
    if (errt_t.error == QJsonParseError::NoError) {
      QJsonObject root = document.object();
      auto data = root.value("data");
      if (data.isObject()) {
        QJsonObject datarot = data.toObject();
        level = datarot.value("level").toInt();
        config->SetBeginGroup("Userinfo");
        config->SetValue("level", QVariant(level));
        config->endGroup();
      }
    }
  }
  reply->deleteLater();
}

void Login::on_finshedVip(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError errt_t{};
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &errt_t);
    config->SetBeginGroup("Userinfo");
    if (errt_t.error == QJsonParseError::NoError) {
      QJsonObject root = document.object();
      QJsonValue data = root.value("data");
      if (data.isObject()) {
        QJsonObject musicrot = data.toObject();
        QJsonValue musicpack = musicrot.value("musicPackage");
        if (musicpack.isObject()) {
          QJsonObject packrot = musicpack.toObject();
          long long expiretime =
              packrot.value("expireTime").toVariant().toLongLong();
          QDateTime data = QDateTime::fromMSecsSinceEpoch(expiretime);
          QString time = data.toString("yyyy-MM-dd").toUtf8();
          qDebug() << time << "\n";
          config->SetValue("vip", QVariant(time));
        }
      }
    } else {
      config->SetValue("vip", QVariant("No VIP"));
    }
    config->endGroup();
  }
  reply->deleteLater();
}

void Login::on_finshedUserMsg(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    QJsonDocument docment = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      QJsonObject rot = docment.object();
      ParseJson(rot);
      Vip();
      Grade();
      emit LoginSucces();
      /*  QJsonValue profile = rot.value("profile");
        if (profile.isObject()) {
        }*/
    }
  }
  reply->deleteLater();
}

//获取国家编码列表
void Login::on_finshedCountriesList(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    QJsonDocument docment = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      QJsonObject root = docment.object();
      QJsonValue data = root.value("data");
      if (data.isArray()) {
        QJsonArray dataAry = data.toArray();
        Config cfg("../config/countrieslist.ini");
        cfg.SetBeginArray("countrieslist");
        int index = 0;
        foreach (const QJsonValue &rhs, dataAry) {
          if (rhs.isObject()) {
            QJsonObject rot = rhs.toObject();
            QString lab = rot.value("label").toString();
            QJsonValue countryList = rot.value("countryList");
            if (countryList.isArray()) {
              QJsonArray countryListArt = countryList.toArray();
              foreach (const QJsonValue &lhs, countryListArt) {
                //只有一个对象，不需要用到循环
                QJsonObject lhsrot = lhs.toObject();
                QString zh = lhsrot.value("zh").toString().toUtf8();
                QString en = lhsrot.value("en").toString();
                QString locale = lhsrot.value("locale").toString();
                QString code = lhsrot.value("code").toString();
                cfg.SetAryIndex(index);
                cfg.SetValue("label", QVariant(lab));
                cfg.SetValue("zh", QVariant(zh));
                cfg.SetValue("en", QVariant(en));
                cfg.SetValue("locale", QVariant(locale));
                cfg.SetValue("code", QVariant(code));
                ++index;
              }
            }
          }
        }
        cfg.endArray();
      }
    } else {
      fprintf(stdout, "Json format error\n");
    }
  }
  reply->deleteLater();
}

void Login::on_finshedQRC_key(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    QJsonDocument docment = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      QJsonObject rot = docment.object();
      QJsonValue data = rot.value("data");
      if (data.isObject()) {
        QJsonObject datarot = data.toObject();
        QRC_KEY = datarot.value("unikey").toString();
        qDebug() << "Key = " << QRC_KEY << "\n";
        //创建二维码

        QString QRC_CreateUrl{
            QString(
                "http://cloud-music.pl-fe.cn/login/qr/create?key=%1&qrimg=0")
                .arg(QRC_KEY)};
        NetQRC_create->get(QNetworkRequest(QRC_CreateUrl));
      }
    } else {
      fprintf(stdout, "json解析失败\n");
    }
  }
  reply->deleteLater();
}

//获取二维码
void Login::on_finshedQRC_create(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    QJsonDocument docment = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      QJsonObject rot = docment.object();
      QJsonValue data = rot.value("data");
      if (data.isObject()) {
        QJsonObject datarot = data.toObject();
        //网页登录
        // QString imgUrl = datarot.value("qrurl").toString();
        QString byt = datarot.value("qrimg").toVariant().toByteArray();
        QPixmap pix;
        // Base64 转成图片
        // 82ec2471-9326-4cc0-8793-bc54c16b8518"
        pix.loadFromData(
            QByteArray::fromBase64(byt.section(",", 1).toLocal8Bit()));
        ui->lab_QRC->setPixmap(pix);
        //保存到本地
        // pix.save("../Userpix/QRC.png");
        //开始计时，一分钟有效时间
        time->start(1000);
        QString checkUrl{
            QString("http://cloud-music.pl-fe.cn/login/qr/check?key=%1")
                .arg(QRC_KEY)};
        NetQRC_check->get(QNetworkRequest(checkUrl));
      }
    } else {
      fprintf(stdout, "json解析失败\n");
    }
  }
  reply->deleteLater();
}

/*
 * 检测是否扫码,轮询此接口可获取二维码扫码状态
 * 800 为二维码过期
 * 801 为等待扫码
 * 802 为待确认
 * 803 为授权登录成功(803 状态码下会返回 cookies)
 */

QJsonDocument docment{};
QJsonObject rot;
void Login::on_finshedQRC_check(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJsonParseError err_t{};
    docment = QJsonDocument::fromJson(reply->readAll(), &err_t);
    if (err_t.error == QJsonParseError::NoError) {
      rot = docment.object();
      int code = rot.value("code").toVariant().toInt();
      qDebug() << "code =" << code << "\n";
      // 800 为二维码过期
      if (800 == code) {
        time->stop();
        int rect = QMessageBox::information(this, "tip",
                                            "二维码不存在或已过期,是否刷新",
                                            QMessageBox::Yes, QMessageBox::No);
        if (rect == QMessageBox::Yes) {
          //从新请求登录二维码
          QDateTime _time = QDateTime::currentDateTime();
          QString Qrc_Key{
              QString("http://cloud-music.pl-fe.cn/login/qr/key?time=%1")
                  .arg(_time.toTime_t())};
          NetQRC_key->get(QNetworkRequest(Qrc_Key));
          reply->deleteLater();
          return;
        }
      }

      if (801 == code) {
        // fprintf(stdout, "Waiting for sweeping code\n");
        // reply->deleteLater();
        return;
      }

      if (802 == code) {
        // fprintf(stdout, "扫码待确认\n");
        reply->deleteLater();
        return;
      }

      if (803 == code) {
        this->close();
        //拿到cookies
        QString cookie = rot.value("cookie").toString();
        NetRequest->setHeader(QNetworkRequest::CookieHeader, QVariant(cookie));
        //获取账号信息
        NetRequest->setUrl(QUrl("http://cloud-music.pl-fe.cn/user/account"));
        NetUserMsg->get(*NetRequest);

        //将cookie放入配置文件
        config->SetBeginGroup("cookie");
        config->SetValue("cookie", QVariant(cookie));
        config->endGroup();
        time->stop();
        fprintf(stdout, "login ok\n");
        ui->stackedWidget->setCurrentIndex(0);
      }
    } else {
      fprintf(stdout, "json解析失败\n");
    }
  }
  reply->deleteLater();
}

void Login::on_pushButton_clicked() {
  QString checkUrl{

      QString("http://cloud-music.pl-fe.cn/login/qr/check?key=%1")
          .arg(QRC_KEY)};
  NetQRC_check->get(QNetworkRequest(checkUrl));
}

void Login::on_QRCexpired() {
  static int n = 0;
  //获取当前时间
  QDateTime _time = QDateTime::currentDateTime();
  if (60 == n) {
    n = 0;
    time->stop();
    int rect =
        QMessageBox::warning(this, "warning", "二维码已过期,是否刷新二维码",
                             QMessageBox::Yes, QMessageBox::No);
    if (rect == QMessageBox::Yes) {
      //从新请求登录二维码
      //将当前时间转为时间戳
      // uint timeT = _time.toTime_t();
      QString Qrc_Key{
          QString("http://cloud-music.pl-fe.cn/login/qr/key?time=%1")
              .arg(_time.toTime_t())};
      NetQRC_key->get(QNetworkRequest(Qrc_Key));
    } else {
      //不处理
    }
  }

  //还没有扫码，持续检测
  ///之后就在这里  
  //NetQRC_check->clearAccessCache();
  QString checkUrl{
      QString("http://cloud-music.pl-fe.cn/login/qr/check?key=%1?time=%2")
          .arg(QRC_KEY)
          .arg(_time.toTime_t())};
  NetQRC_check->get(QNetworkRequest(checkUrl));

  //++n;
   fprintf(stdout, "coming cheach = %d\n", ++n);
}