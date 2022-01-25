#ifndef CONFIG_H
#define CONFIG_H
#include <QList>
#include <QNetworkCookie>
#include <QString>
class QNetworkRequest;
class QNetworkCookie;
/*
 *程序配置文件
 */
class QSettings;
class QVariant;
class QStringList;

class Config {
public:
  Config(QString qstrfilename = "../config/config.ini");
  ~Config();
  void setPath(const QString path);
  void SetValue(const QString key, const QVariant val);
  void SetBeginGroup(const QString key);
  void SetBeginArray(const QString key);
  int SetBeginReadAry(const QString &str);
  void SetAryIndex(const int index);
  void endArray();
  void endGroup();
  void DelCongif(const QStringList strlist);
  void DelCongif(const QString str);
  void ChangData(const QString &str, const QVariant &val);
  QByteArray GetCookies();
  QStringList GetAllValue(const QString key);
  QString GetValue(const QString &str);
  QNetworkRequest *setCookies();
  QNetworkRequest *getRequest() { return request; }
  QSettings *getsetting() { return m_psetting; }

private:
  QByteArray cookie{};
  QString m_qstrFileName;
  QSettings *m_psetting;
  QNetworkRequest *request;
  QList<QNetworkCookie> cookies;
};

#endif // CONFIG_H
