#include "loadJson.h"
#include <QJsonObject>
//封装了JSONd支持的数据类型
#include <QJsonParseError>
#include <QJsonValue>
Personal_information::Personal_information()
    : ID{}, UserName{}, BackgroundUrl{}, AvatarUrl{}, Fans{}, Follows{},
      playlistCount{} {}

Personal_information::~Personal_information() {}


ParseLoadJson::ParseLoadJson() {}

ParseLoadJson::~ParseLoadJson() {}
#include <QbyteArray>
//对json的操作
void ParseLoadJson::WriteJson() {
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
    //obj.insert("cookie", QString(User.cookie));
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
    perror("User.json");
  }
}

bool ParseLoadJson::ReadJson() {

  QFile json{"../Json/User.json"};
  if (json.open(QIODevice::ReadOnly)) {
    QByteArray byte = json.readAll();
    QJsonParseError err_rpt{};
    QJsonDocument document = QJsonDocument::fromJson(byte, &err_rpt);
    if (err_rpt.error == QJsonParseError::NoError) {
      QJsonObject rootobj = document.object();
      ParseJson(rootobj);
    }
    json.close();
    return true;
  }
  return false;
}

bool ParseLoadJson::ParseJson(QJsonObject &rootobj) {
  //获取用户信息
  QJsonValue profileobj = rootobj.value("profile");
  //判断这个profile对象是否是object
  if (profileobj.isObject()) {
    //将其转换成object
    auto profile = profileobj.toObject();
    //更具key获取对应的value
    User.ID = profile.value("userId").toInt();
    User.UserName = profile.value("nickname").toString();
    User.AvatarUrl = profile.value("avatarUrl").toString();
    User.BackgroundUrl = profile.value("backgroundUrl").toString();
    User.Fans = profile.value("followeds").toInt();
    User.Follows = profile.value("follows").toInt();
    User.playlistCount = profile.value("playlistCount").toInt();
    //conf.SetBeginGroup("userinfo");
   
    return true;
  }
  //#ifdef _DEBUG
  //  qDebug() << User.ID << '\n'
  //           << User.UserName << '\n'
  //           << User.AvatarUrl << '\n'
  //           << User.BackgroundUrl << '\n'
  //           << User.Fans << '\n'
  //           << User.Follows << '\n'
  //           << User.playlistCount << "\n"
  //           << User.cookie << "\n";
  //#endif

  return false;
}
