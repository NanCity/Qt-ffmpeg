/*
 * @Author: your name
 * @Date: 2021-09-20 12:09:20
 * @LastEditTime: 2021-10-12 00:19:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Music\main.cpp
 */
#include "music.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QTextCodec>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/images/logo.ico"));    // Application icon
  a.setStyle(QStyleFactory::create("macintosh")); //设置窗口风格
  //设置中文字体
  a.setFont(QFont("Microsoft Yahei", 9));
  //设置中文编码
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
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
#endif

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
