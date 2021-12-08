#include "lyric.h"
#include "ui_lyric.h"
lyric::lyric(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lyric)
{
    ui->setupUi(this);
    QPixmap map;
    map.load("E:\\QTCode\\Music\\Qss\\小舞.jpg");
    ui->lab_AlbumPic->setPixmap(map);
}

lyric::~lyric()
{
    delete ui;
}
