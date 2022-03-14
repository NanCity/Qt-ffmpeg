#include "music.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStyleFactory>
#include <QSplashScreen>
#include <thread>
//#include <QTextCodec>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	//窗体图标
	//    a.setWindowIcon(QIcon(""));
	a.setStyle(QStyleFactory::create("macintosh")); //设置窗口风格
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
	//启动动画
	//QPixmap pix("F:/QTCode/Music/photowall/5.png");
	//QSplashScreen splash(pix);
	//splash.show();
	//splash.showMessage("正在启动程序", Qt::AlignTop | Qt::AlignRight, Qt::red);
	//a.processEvents();
	std::thread thd{ []() {
		// system() 函数会调用fork()函数，会阻塞运行，这里另开一个线程，避免阻塞
		const char* cmd("node ../NeteaseCloudMusicApi/app.js");
		 system(cmd);
		} };
	//不阻塞
	thd.detach();


	Music w;
	w.show();
	return a.exec();

}