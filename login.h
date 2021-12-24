#ifndef LOGIN_H
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
class QRegExp;
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

class Login : public QDialog {
  Q_OBJECT

public:
  explicit Login(QWidget *parent = nullptr);
  virtual ~Login();
  void WriteJson() const;
  void ReadJson();
  bool ParseJson(QJsonValue &value);
private slots:
  void on_btn_signin_clicked();
  void replyFinished(QNetworkReply *);
  void checkBoxState(int state);
  bool checkInput(QString &str, const QRegExp regx, const int n);

private:
  Ui::Login *ui;
  QString phone{};
  QString pwd{};
  Personal_information User;
  QNetworkRequest *NetRequest;
  //发送网络请求和接受应答
  QNetworkAccessManager *NetManager;
};

#endif // LOGIN_H
