#include "config.h"
#include <QDateTime>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTextCodec>
#include <QVariant>
Config::Config(QString qstrfilename) {
  m_qstrFileName = qstrfilename;
  m_psetting = new QSettings(qstrfilename, QSettings::IniFormat);
  m_psetting->setIniCodec(QTextCodec::codecForName("GBK"));
  request = new QNetworkRequest();
}

Config::~Config() {
  delete m_psetting;
  m_psetting = nullptr;
  delete request;
  request = nullptr;
}

void Config::setPath(const QString path) {
  m_qstrFileName = path;
  m_psetting = new QSettings(m_qstrFileName, QSettings::IniFormat);
  m_psetting->setIniCodec(QTextCodec::codecForName("GBK"));
}

void Config::SetValue(const QString key, const QVariant val) {
  //写入文件
  m_psetting->setValue(key, val);
}

void Config::SetBeginGroup(const QString key) {
  // 设置当前节名
  m_psetting->beginGroup(key);
}

void Config::SetBeginArray(const QString key) {
  m_psetting->beginWriteArray(key);
}

int Config::SetBeginReadAry(const QString &str) {
  return m_psetting->beginReadArray(str);
}

void Config::SetAryIndex(const int index) { m_psetting->setArrayIndex(index); }

void Config::endArray() { m_psetting->endArray(); }

void Config::endGroup() {
  // 结束当前节的操作

  m_psetting->endGroup();
}

void Config::DelCongif(const QStringList strlist) {
  for (int i = 0; i != strlist.length(); ++i) {
    m_psetting->remove(strlist.at(i));
  }
}

void Config::DelCongif(const QString str) { m_psetting->remove(str); }

void Config::ChangData(const QString &str, const QVariant &val) {
  m_psetting->setValue(str, val);
}

QByteArray Config::GetCookies() {
  m_psetting->beginGroup("cookie");
  QByteArray byte = m_psetting->value("cookie", "null").toByteArray();
  m_psetting->endGroup();
  return byte;
}

QStringList Config::GetAllValue(const QString key) {
  m_psetting->beginGroup(key);
  QStringList list{};
  for (int i = 0; i != m_psetting->allKeys().length(); ++i) {
    list.append(m_psetting->value(QString("%1").arg(i), "null").toString());
  }
  m_psetting->endGroup();

  return list;
}

QString Config::GetValue(const QString &str) {
  return m_psetting->value(str).toString();
}

QNetworkRequest *Config::setCookies() {
  m_psetting->beginGroup("cookie");
  QByteArray byte = m_psetting->value("cookie", "null").toByteArray();
  m_psetting->endGroup();

  if (byte.isEmpty()) {
    fprintf(stdout, "Error: %s  %d\n", __FILE__, __LINE__);
    return request;
  } else {
    //解析cookies
    cookies = QNetworkCookie::parseCookies(byte);
    //向请求头里加入cookies
    QVariant var{};
    var.setValue(cookies);
    //设置请求头
    request->setHeader(QNetworkRequest::CookieHeader, var);
    return request;
  }
}