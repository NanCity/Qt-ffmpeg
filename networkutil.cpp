#include "networkutil.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetWorkUtil::Private {
public:
  Private(NetWorkUtil *q) : manager(new QNetworkAccessManager(q)) {}
  QNetworkAccessManager *manager;
};

NetWorkUtil::NetWorkUtil(QObject *parent) : QObject(parent) {
  d = new NetWorkUtil::Private(this);
  connect(d->manager, &QNetworkAccessManager::finished, this,
          &NetWorkUtil::finished);
}

QNetworkReply *NetWorkUtil::get(const QString &url) {
  return d->manager->get(QNetworkRequest(QUrl(url)));
}

NetWorkUtil *NetWorkUtil::instance() {
  static NetWorkUtil networkUtil;
  return &networkUtil;
}

NetWorkUtil::~NetWorkUtil() {
  delete d;
  d = nullptr;
}
