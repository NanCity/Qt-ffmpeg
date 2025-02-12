﻿#include "local_and_download.h"
#include "ui_local_and_download.h"
#include"base.h"
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>

Local_and_Download::Local_and_Download(QWidget *parent)
    : QWidget(parent), ui(new Ui::Local_and_Download) {
  ui->setupUi(this);
  base = new Base(ui->tableWidget);
  InitTableHeader();
  ui->tab_Widget->setTabText(0, tr("本地音乐"));
  ui->tab_Widget->setTabText(1, tr("下载与管理"));
  ui->tableWidget->setWindowFlags(Qt::CustomizeWindowHint |
                                  Qt::FramelessWindowHint);
  //允许QTableWidget接收QWidget::customContextMenuRequested()信号。
  ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  //初始化菜单按钮
  Menu();
  //关联本地搜索
  connect(ui->line_search, &QLineEdit::textChanged, this,
          [=]() { base->SerachData(ui->line_search->text()); });

  //发送播放信号
  connect(play, &QAction::triggered, this, [=]() {
    int currow = ui->tableWidget->currentRow();
    emit t_play(currow);
  });

  //发送播放下一首信号
  connect(nextplay, &QAction::triggered, this, [=]() {
    int currow = ui->tableWidget->currentRow();
    emit t_nextplay(currow);
  });

  //从列表中删除
  connect(Del, &QAction::triggered, this, [=]() {
    int currow = ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(currow);
    //lists.removeAt(currow);
    qDebug() << "currow: " << currow << '\n';
    ui->lab_MusicSum->setText(tr("本地共有 %1 首歌曲").arg(lists.length()));
    emit t_delete(currow);
  });
}

Local_and_Download::~Local_and_Download() { delete ui; }

/*
 * 打开本地文件
 * 使用函数avformat_open_input打开文件，结封装。
 * 使用函数avformat_find_stream_info查找并添加流信息到Format上下文中。
 * 使用函数av_dict_get获取文件中的字典信息。
 */
void Local_and_Download::on_btn_openFile_clicked() {
  if (!lists.isEmpty()) {
    base->DelTableWidgetRow();
  }
  lists = QFileDialog::getOpenFileNames(
      this, tr("Ctrl+A select All"), tr("E:/Music/"), tr("mp3(*.mp3 *.flac)"));
  if (lists.isEmpty()) {
    QMessageBox::information(this, tr("Error"), tr("文件打开失败"),
                             QMessageBox::Yes);
    return;
  }

  /*
   * 解码
   */
  for (int index = 0; index != lists.length(); ++index) {
    //获取MP3 Tag 标签
    QStringList val = code.DeCodeTag(lists.at(index).toStdString().c_str());
    base->InsertDataInfoTableWidget(val, index);
  }
  ui->lab_MusicSum->setText(tr("本地共有 %1 首歌曲").arg(lists.length()));
  emit t_loaded(lists);
  //播放
}

void Local_and_Download::on_tableWidget_customContextMenuRequested(
    const QPoint &pos) {
  qDebug() << "on_tableWidget_customContextMenuRequested\n";
  if (ui->tableWidget->itemAt(pos) != nullptr) {
    menu->addActions(listAct);
    //不加这一句，显示位置父控件里面 QCursor::pos()获取当前鼠标的位置
    menu->exec(QCursor::pos());
  }
}

void Local_and_Download::Menu() {
  menu = new QMenu(this);

  play = new QAction(QIcon(":/images/bottom/btn_play_h.png"), tr("Play"), menu);

  nextplay = new QAction(QIcon(":/images/bottom/btn_single_h.png"),
                         tr("Next Play"), menu);

  Del = new QAction(QIcon(":/images/btn_delete_h.png"),
                    tr("Delete from the list"), menu);
  listAct.push_back(play);
  listAct.push_back(nextplay);
  listAct.push_back(menu->addSeparator()); //设置间隔期)
  listAct.push_back(Del);
}

void Local_and_Download::InitTableHeader() {
  QStringList HorizontalHeaderItem{QObject::tr("音乐标题"), QObject::tr("歌手"),
                                   QObject::tr("专辑"), QObject::tr("时长"),
                                   QObject::tr("大小")};
  //获取列数
  int column = HorizontalHeaderItem.count();
  ui->tableWidget->setColumnCount(column);
  //设置行高
  ui->tableWidget->setLineWidth(10);
  for (int x = 0; x != column; ++x) {
    ui->tableWidget->setHorizontalHeaderItem(
        x, new QTableWidgetItem(HorizontalHeaderItem.at(x)));
  }
}


QTableWidget *Local_and_Download::getTable() { return ui->tableWidget; }
