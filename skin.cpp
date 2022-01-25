#include "skin.h"
#include "ui_skin.h"
#include "LoadFileQss.h"
#include <QDebug>
skin::skin(QWidget *parent) : QDialog(parent), ui(new Ui::skin) {
  ui->setupUi(this);
  //设置无边框
  this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  init();
}

skin::~skin() { delete ui; }

void skin::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  this->close();
}

void skin::init() {
  ui->tabWidget->setTabText(0, "主题");
  ui->tabWidget->setTabText(1,"纯色");
}

//void skin::OpenQssFile(QString path) {
//  QFile qssfile{path};
//  if (qssfile.open(QIODevice::ReadOnly)) {
//    qssfile.readAll();
//    emit setThem(qssfile.readAll());
//  } else {
//    perror(path.toStdString().data());
//  }
//  qssfile.close();
//}

void skin::on_btn_black_clicked() { LoadFileQss::LoadQss(":/style/skin/black.css"); }

void skin::on_btn_red_clicked() {
  LoadFileQss::LoadQss(":/style/skin/red.css");
}

void skin::on_btn_pin_clicked() {
  LoadFileQss::LoadQss(":/style/skin/pink.css");
}

void skin::on_btn_blue_clicked() {
  LoadFileQss::LoadQss(":/style/skin/blue.css");
}

void skin::on_btn_green_clicked() {
  LoadFileQss::LoadQss(":/style/skin/green.css");
}

void skin::on_btn_gold_clicked() {
  LoadFileQss::LoadQss(":/style/skin/gold.css");
}
