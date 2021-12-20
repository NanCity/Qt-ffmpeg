#include "search.h"
#include "ui_search.h"

Search::Search(QWidget *parent) : QWidget(parent), ui(new Ui::Search) {
  ui->setupUi(this);
this->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);

  ui->scrollArea->setWindowFlags(Qt::CustomizeWindowHint  | Qt::FramelessWindowHint);
  ui->tabWidget->setWindowFlags(Qt::CustomizeWindowHint   | Qt::FramelessWindowHint);
  ui->table_playlist->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  InitPlayListTabWiget();
}

Search::~Search() { delete ui; }

void Search::InitPlayListTabWiget() {
    ui->table_playlist->setStyleSheet("QTabBar::tab{width:120px;height:30px;");
}

void Search::on_btn_playall_clicked() {}
