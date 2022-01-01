#include "desktoplyrics.h"
#include "ui_desktoplyrics.h"
#include <QDebug>
DesktopLyrics::DesktopLyrics(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DesktopLyrics)
{
    ui->setupUi(this);
    ui->lab_lastrow->setStyleSheet("color:pink");
    ui->lab_nextrow->setStyleSheet("color:blue");
}

DesktopLyrics::~DesktopLyrics()
{
    delete ui;
}

void DesktopLyrics::SetLastlyric(QString str)
{
    str.remove("\\n");
    ui->lab_lastrow->setText(str);
}

void DesktopLyrics::SetNextlyric(QString str)
{
     str.remove("\\n");
     ui->lab_nextrow->setText(str);
}

void DesktopLyrics::on_btn_close_clicked()
{
    this->close();
}

