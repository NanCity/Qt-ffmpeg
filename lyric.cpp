#include "lyric.h"
#include "ui_lyric.h"
#include <QDebug>
#include <QMenu>
#include <QAction>

lyric::lyric(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::lyric)
{
	ui->setupUi(this);
	this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	QPixmap map;
	ui->lab_artist->setStyleSheet("font:bold 15px;");
	map.load("E:\\QTCode\\Music\\Qss\\小舞.jpg");
	ui->lab_AlbumArt->setPixmap(map);
	ui->lab_AlbumArt->setStyleSheet("QLabel::#ui->lab_AlbumArt{ background-color: transparent; border-radius: 10px}");
}

lyric::~lyric()
{
	delete ui;
}


void lyric::setMessage(QImage& img, QString Art, QString& title)
{
	QPixmap pixmap{ QPixmap::fromImage(img) };
	//设置自适应图片大小
	pixmap.scaled(ui->lab_AlbumArt->size(), Qt::KeepAspectRatio);
	//setScaledContents：设置label的属性scaledContents，这个属性的作用是允许（禁止）label缩放它的内容充满整个可用的空间。
	ui->lab_AlbumArt->setScaledContents(true);
	ui->lab_AlbumArt->setPixmap(pixmap);
	ui->lab_AlbumArt->setStyleSheet("border: 0px solid ");
	//ui->lab_lyric->setim
	//设置文字居中,且空间中垂直中心

	ui->lab_AlbumArt->setStyleSheet("QLabel::#ui->lab_AlbumArt{ background-color: transparent;  border-radius: 10px}");

	ui->lab_lyric->setStyleSheet("background-color: transparent;");
	ui->lab_artist->setText(QString("%1\n%2").arg(title).arg(Art));
	ui->lab_artist->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}