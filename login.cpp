#include "login.h"
#include "ui_login.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QMessageBox>
#include <QRegExp>
#include <QRegExpValidator>
Login::Login(QWidget *parent) : QDialog(parent), ui(new Ui::Login) {
  ui->setupUi(this);

  if (ui->checkBox->checkState() == Qt::Checked) {
    phone = ui->line_phoneID->text();
    pwd = ui->line_word->text();
  }
  //处于输入状态的时候，是正常显示字符。 输入完毕之后,使用Password形式隐藏字符
  ui->line_word->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  NetManager = new QNetworkAccessManager{this};
  NetRequest = new QNetworkRequest{};
  //请求结束以后调用该槽
  connect(NetManager, &QNetworkAccessManager::finished, this,
          &Login::replyFinished);
  connect(ui->checkBox, &QCheckBox::stateChanged, this, &Login::checkBoxState);
  // connect(ui->line_phoneID, &QLineEdit::returnPressed, this,
  // &Login::checkInput);
}

Login::~Login() {
  delete ui;
  delete NetRequest;
  NetRequest = nullptr;
}

//读取文件中的信息
void Login::ReadJson() {
  QFile readFile{"../Json/User.json"};
  if (readFile.open(QIODevice::ReadOnly)) {
    QByteArray byte_arry{readFile.readAll()};
    QJsonParseError err_rpt{};
    QJsonDocument document = QJsonDocument::fromJson(byte_arry, &err_rpt);
    if (err_rpt.error == QJsonParseError::NoError) {
      QJsonObject rootobj = document.object();
      QJsonValue profile = rootobj.value("profile");
      ParseJson(profile);
    } else {
      Error(tr("Json format error"));
      return;
    }

    readFile.close();
  } else {
    Error("Can't open User.json file.");
    return;
  }
}

//将json写入文件
void Login::WriteJson() const {
  QFile in_file{"../Json/User.json"};
  if (in_file.open(QIODevice::WriteOnly)) {
    QJsonObject obj;
    obj.insert("userId", User.ID);
    obj.insert("nickname", User.UserName);
    obj.insert("backgroundUrl", User.BackgroundUrl);
    obj.insert("avatarUrl", User.AvatarUrl);
    obj.insert("followeds", User.Fans);
    obj.insert("follows", User.Follows);
    obj.insert("playlistCount", User.playlistCount);
    //第二种方式
    // obj["ID"] = User.ID;
    // obj["UserName"] = User.UserName;
    // obj["BackgroundUrl"] = User.BackgroundUrl;
    // obj["AvatarUrl"] = User.AvatarUrl;
    // obj["Fans"] = User.Fans;
    // obj["Follows"] = User.Follows;
    // obj["playlistCount"] = User.playlistCount;

    //第一层的 josn 对象
    // QJsonObject UserJson;
    static QJsonObject UserJson{};
    UserJson["profile"] = obj;
    QJsonDocument document{UserJson};
    //转换成json格式，然后写入文件 QJsonDocument::Indented 防止中文乱码
    QByteArray arry = document.toJson(QJsonDocument::Indented);
    //写入文件
    in_file.write(arry);
    in_file.close();
  } else {
    Error("Can't open User.json file.");
    return;
  }
}

bool Login::ParseJson(QJsonValue &profile) {
  //判断这个profile对象是否是object
  if (profile.isObject()) {
    //将其转换成object
    QJsonObject profileobj = profile.toObject();
    //更具key获取对应的value
    User.ID = profileobj.value("userId").toInt();
    User.UserName = profileobj.value("nickname").toString();
    User.AvatarUrl = profileobj.value("avatarUrl").toString();
    User.BackgroundUrl = profileobj.value("backgroundUrl").toString();
    User.Fans = profileobj.value("followeds").toInt();
    User.Follows = profileobj.value("follows").toInt();
    User.playlistCount = profileobj.value("playlistCount").toInt();
#ifdef _DEBUG
    qDebug() << User.ID << '\n'
             << User.UserName << '\n'
             << User.AvatarUrl << '\n'
             << User.BackgroundUrl << '\n'
             << User.Fans << '\n'
             << User.Follows << '\n'
             << User.playlistCount << "\n";
#endif
    return true;
  }
  return false;
}

//登陆
void Login::on_btn_signin_clicked() {
  //验证手机号码 1,2用于区分是密码错误还是手机号码错误
  static QRegExp regx_phone(
      "^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$");
  checkInput(ui->line_phoneID->text(), regx_phone, 1);
  //验证密码 8-16位且必须为数字、大小写字母或符号中至少2种
  static QRegExp regx_pwd{
      "^(?![\\d]+$)(?![a-zA-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$"};
  checkInput(ui->line_word->text(), regx_pwd, 2);

  QString phone = ui->line_phoneID->text();
  QString psw = ui->line_word->text();
  //判断一下网络状态， 如果为NotAccessible重新设置一下
  if (NetManager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
    NetManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  //以get方式发送一个网络请求
  NetManager->get(QNetworkRequest(
      QString(
          "http://cloud-music.pl-fe.cn/login/cellphone?phone=%1&password=%2")
          .arg(phone)
          .arg(psw)));
}

void Login::replyFinished(QNetworkReply *reply) {
  QString all = reply->readAll();
  QByteArray byte_array{};
  byte_array.append(all);
  QJsonParseError err_rpt{};
  // QJsonDocument封装了完整的json文档
  QJsonDocument document = QJsonDocument::fromJson(byte_array, &err_rpt);
  QJsonObject rootobj = document.object();
  if (err_rpt.error == QJsonParseError::NoError) {
    for (auto it = rootobj.begin(); it != rootobj.end(); ++it) {
      QJsonValue profile = rootobj.value("profile");
      //判断这个profile对象是否是object
      // if (profile.isObject()) {
      //	//将其转换成object
      //	QJsonObject	profileobj = profile.toObject();
      //	//更具key获取对应的value
      //	User.ID = profileobj.value("userId").toInt();
      //	User.UserName = profileobj.value("nickname").toString();
      //	User.AvatarUrl = profileobj.value("avatarUrl").toString();
      //	User.BackgroundUrl =
      // profileobj.value("backgroundUrl").toString(); 	User.Fans =
      // profileobj.value("followeds").toInt(); 	User.Follows =
      // profileobj.value("follows").toInt(); 	User.playlistCount =
      // profileobj.value("playlistCount").toInt();
      //	//qDebug() << User.ID << '\n' << User.UserName << '\n' <<
      // User.AvatarUrl
      //	//	<< '\n' << User.BackgroundUrl << '\n' << User.Fans <<
      //'\n' << User.Follows
      //	//	<< '\n' << User.playlistCount << "\n";
      //	WriteJson();
      //	break;
      //}
      if (ParseJson(profile)) {
        WriteJson();
        return;
      }
    }
  } else {
    Error(tr("Json format error"));
    return;
  }
}

void Login::checkBoxState(int state) {
  switch (state) {
  case Qt::Unchecked: //未勾选
    break;
  case Qt::Checked: //已勾选
    ReadJson();     //读取已保存的json文件
    ui->checkBox->setCheckState(Qt::Checked);
    ui->line_phoneID->setText(phone);
    ui->line_word->setText(pwd);
    break;
  default:
    break;
  }
}

bool Login::checkInput(QString str, const QRegExp regx, const int n) {
  QRegExpValidator pReg{regx, 0};
  int pos = 0;
  QValidator::State res = pReg.validate(str, pos);
  switch (n) {
  case 1:
    //输入的格式不对
    if (!(QValidator::Acceptable == res)) {
      if (QMessageBox::warning(this, tr("Warning"),
                               tr("Please enter an 11-digit mobile phone\n")) ==
          QMessageBox::Ok) {
        ui->line_phoneID->clear();
        ui->line_word->clear();
        return true;
      }
    }
    break;
  case 2:
    if (!(QValidator::Acceptable == res)) {
      if (QMessageBox::warning(this, tr("Warning"), tr("Password Error\n")) ==
          QMessageBox::Ok) {
        ui->line_word->clear();
        return true;
      }
    }
    break;
  default:
    break;
  }
  return false;
}

Personal_information::Personal_information()
    : ID{}, UserName{}, BackgroundUrl{}, AvatarUrl{}, Fans{}, Follows{},
      playlistCount{} {}

Personal_information::~Personal_information() {}
