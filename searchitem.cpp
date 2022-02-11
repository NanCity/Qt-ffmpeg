#include "searchitem.h"
#include "ui_searchitem.h"

searchItem::searchItem(QWidget *parent)
    : QWidget(parent), ui(new Ui::searchItem) {
  ui->setupUi(this);
  ui->lab_content->setStyleSheet("color:#999999");
}

searchItem::~searchItem() { delete ui; }

QLabel *searchItem::getHot() { return ui->lab_hot; }

QString searchItem::getName() { return ui->lab_name->text(); }

void searchItem::setNumber(const int num) {
  ui->lab_number->setText(QString("%1").arg(num));
}

void searchItem::setName(const QString name) { ui->lab_name->setText(name); }

void searchItem::setHot(QPixmap pix) { ui->lab_hot->setPixmap(pix); }

void searchItem::setScore(const int socre) {
  ui->lab_score->setText(QString("%1").arg(socre));
}

void searchItem::setContent(QString mes) { ui->lab_content->setText(mes); }

void searchItem::setLabNumColor(bool bol) {
  if (bol == false) {
    ui->lab_number->setStyleSheet("color:#c1c1c1");
    ui->lab_name->setStyleSheet("color:##b0b0b0");
  } else {
    ui->lab_number->setStyleSheet("color:red");
    ui->lab_name->setStyleSheet(
        "font-family:Microsoft YaHei;font-size:13px; color:#313131;");
  }
}
