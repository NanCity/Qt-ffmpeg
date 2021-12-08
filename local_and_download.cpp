
#include "base.h"
#include <qtablewidget.h>
#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "local_and_download.h"
#include "ui_local_and_download.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>

Local_and_Download::Local_and_Download(QWidget *parent)
    : QWidget(parent), ui(new Ui::Local_and_Download) {
  ui->setupUi(this);
  //初始化tableWidget
  InitTableWidget(ui->tableWidget);

  //关联本地搜索
  connect(ui->line_search, &QLineEdit::textChanged, this, [=]() {
    Base::SerachData(ui->tableWidget, ui->line_search->text());
  });
  // 在父窗体构造函数中允许QTableWidget接收QWidget::customContextMenuRequested()
  ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
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
    Base::DelTableWidgetRow(ui->tableWidget);
  }
  lists = QFileDialog::getOpenFileNames(
      this, tr("Ctrl+A select All"), tr("D:/Music/"), tr("mp3(*.mp3 *.flac)"));
  if (lists.isEmpty()) {
    QMessageBox::information(this, tr("Error"), tr("文件打开失败"),
                             QMessageBox::Yes);
    return;
  }

  /*
   * 解码
   */

  AudioDeCode code{};
  for (int index = 0; index != lists.length(); ++index) {
    //获取MP3 Tag 标签
    QStringList val = code.DeCodeTag(lists.at(index).toStdString().c_str());
    ui->tableWidget->insertRow(index);
    InsertDataInfoTableWidget(val, index);
  }
  ui->lab_MusicSum->setText(tr("本地共有 %1 首歌曲").arg(lists.length()));
  //播放
}

void Local_and_Download::InsertDataInfoTableWidget(const QStringList &value,
                                                   const int index) {
  for (int row = 0; row != 5; ++row) {
    ui->tableWidget->setItem(index, row, new QTableWidgetItem(value.at(row)));
  }
}

// QStringList &Local_and_Download::getMusicPath() { return lists; }

void Local_and_Download::on_btn_LocalMusic_clicked() {
  //  ui->tableWidget->show();
}

QTableWidget *Local_and_Download::getTable() { return ui->tableWidget; }

//鼠标点击右键
void Local_and_Download::on_tableWidget_customContextMenuRequested(
    const QPoint &pos) {
  TableMenu(ui->tableWidget);
  qDebug() << "**********************Clicked Right Mouse\n";
  //
}
