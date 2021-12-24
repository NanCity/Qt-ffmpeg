#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H
#include <QObject>
class QNetworkReply;
class NetWorkUtil : public QObject {
  Q_OBJECT
public:
  static NetWorkUtil *instance();
  QNetworkReply *get(const QString &url);
  virtual ~NetWorkUtil();
signals:
  void finished(QNetworkReply *reply);
public slots:

private:
  explicit NetWorkUtil(QObject *parent = nullptr);
  NetWorkUtil(NetWorkUtil &) = delete;
  NetWorkUtil &operator=(NetWorkUtil nwu) = delete;

private:
  class Private;
  friend class Private;
  Private *d;
};
#endif // NETWORKUTIL_H
