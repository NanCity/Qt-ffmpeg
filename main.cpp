#include "music.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>
//#include <QTextCodec>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setStyle(QStyleFactory::create("macintosh"));  //设置窗口风格
                                                   //设置中文编码
  /* #if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
   #if _MSC_VER
     QTextCodec *codec = QTextCodec::codecForName("GBK");
   #else
     QTextCodec *codec = QTextCodec::codecForName("UTF-8");
   #endif
     QTextCodec::setCodecForLocale(codec);
     QTextCodec::setCodecForCStrings(codec);
     QTextCodec::setCodecForTr(codec);
   #else
     QTextCodec *codec = QTextCodec::codecForName("UTF-8");
     QTextCodec::setCodecForLocale(codec);
   #endif*/

  //加载需要用到的图标
  QFile file(":/style/qss/setImages.css");
  if (file.open(QIODevice::ReadOnly)) {
    QString qss = file.readAll();
    a.setStyleSheet(qss); //设置图标
  }
  //关闭文件
  file.close();
  Music w;
  w.show();
  return a.exec();
}
