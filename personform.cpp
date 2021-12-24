#include "personform.h"
#include "ui_personform.h"

PersonForm::PersonForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::PersonForm) {
  ui->setupUi(this);
  //设置无边框
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  ui->listWidget->setStyleSheet("background-color:transparent;");

  this->setStyleSheet("background-color:pink");
  initListWidget();
}

PersonForm::~PersonForm() { delete ui; }

void PersonForm::initListWidget() {

  // ui->listWidget->item(0)->setIcon(QIcon("border-image:
  // url(:/images/btn_openfile_n.png)"));
  // ui->listWidget->item(2)->setIcon(QIcon("border-image:
  // url(:/images/btn_openfile_n.png)"));
  // ui->listWidget->item(3)->setIcon(QIcon("border-image:
  // url(:/images/btn_openfile_n.png)"));
  // ui->listWidget->item(5)->setIcon(QIcon("border-image:
  // url(:/images/btn_openfile_n.png)"));
  // ui->listWidget->item(6)->setIcon(QIcon("border-image:
  // url(:/images/btn_openfile_n.png)"));
}

void PersonForm::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  this->close();
}
