#ifndef LOADFILEQSS_H
#define LOADFILEQSS_H
#include <QApplication>
#include <QFile>
class LoadFileQss {
public:
  static void LoadQss(const QString &fileName) {
    QFile Qss(fileName);
    if (Qss.open(QIODevice::ReadOnly)) {
      qApp->setStyleSheet(Qss.readAll());
      Qss.close();
    }
  }
};

#endif // LOADFILEQSS_H