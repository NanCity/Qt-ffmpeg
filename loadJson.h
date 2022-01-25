#ifndef LOADJSON_H
#define LOADJSON_H
//已弃用
#include <QFile>
class QJsonObject;
class QJsonValue;

class Personal_information {
public:
  /*用户ID*/
  int ID;
  /*用户名*/
  QString UserName;
  /*背景图片*/
  QString BackgroundUrl;
  /*用户头像*/
  QString AvatarUrl;
  /*用户粉丝*/
  int Fans;
  /*关注人数*/
  int Follows;
  /*歌单*/
  int playlistCount;

public:
  Personal_information();
  ~Personal_information();
};

class ParseLoadJson {
public:
  ParseLoadJson();
  ~ParseLoadJson();
  Personal_information User;
  void WriteJson();
  bool ReadJson();
  bool ParseJson(QJsonObject &rootobj);
  QByteArray getCookie() { return QByteArray(); };
};

#endif // LOADJSON_H
