#include "base.h"
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QTableWidget>
#include <qtablewidget.h>
Base::Base(QTableWidget *parent) : tab{parent} {
  //允许QTableWidget接收QWidget::customContextMenuRequested()信号。
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  InitTableWidget();
}

Base::~Base() {}

//初始化TableWidget
void Base::InitTableWidget() {
  //设置无边框
  // ui->tableWidget->setFrameShape(QFrame::NoFrame);
  //设置触发条件：不可编辑
  tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tab->setSortingEnabled(false); //启动排序
                                 // item 水平表头自适应大小

  tab->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tab->horizontalHeader()->setDefaultSectionSize(35);
  // item 垂直表头自适应大小
  // ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QStringList HorizontalHeaderItem{QObject::tr("音乐标题"), QObject::tr("歌手"),
                                   QObject::tr("专辑"), QObject::tr("时长"),
                                   QObject::tr("大小")};
  //获取列数
  int column = HorizontalHeaderItem.count();
  tab->setColumnCount(column);
  //设置行高
  tab->setLineWidth(10);
  for (int x = 0; x != column; ++x) {
    tab->setHorizontalHeaderItem(
        x, new QTableWidgetItem(HorizontalHeaderItem.at(x)));
  }
  tab->setAlternatingRowColors(true);
  tab->setSelectionBehavior(
      QAbstractItemView::SelectRows); //设置选择行为时每次选择一行
}

void Base::DelTableWidgetRow() {
  for (int index = tab->rowCount(); index >= 0; --index) {
    tab->removeRow(index);
  }
}

void Base::SerachData(QString search_data) {
  const int curRow = tab->rowCount();
  if ("" == search_data) {
    for (int i = 0; i != curRow; i++) {
      tab->setRowHidden(i, false);
    }
  } else {
    //列出所有的条件的cell索引
    QList<QTableWidgetItem *> item =
        tab->findItems(search_data, Qt::MatchContains);
    for (int i = 0; i != curRow; i++) {
      //隐藏所有行
      tab->setRowHidden(i, true);
    }
    if (!item.isEmpty()) {
      //打印查询到的结果
      for (int i = 0; i != item.count(); i++) {
        tab->setRowHidden(item.at(i)->row(), false);
      }
    }
  }
}



/*******************初始化QAudioOutPut***********************/
// Player::Player() : data_pcm(0) {
//  this->open(QIODevice::ReadOnly); // 为了解决QIODevice::read (QIODevice):
//                                   // device not open.
//  initAudio();
//  AudioOutPut = new QAudioOutput(initAudio(), this);
//  len_written = 0;
//  connect(AudioOutPut, &QAudioOutput::stateChanged, this,
//          &Player::handleStateChanged);
//}
//
// void Player::handleStateChanged(QAudio::State state) {
//  switch (state) {
//  case QAudio::ActiveState:
//    qDebug() << "playing....... \n";
//    break;
//  case QAudio::SuspendedState:
//    //暂停
//    qDebug() << "suspend....... \n";
//    break;
//  case QAudio::StoppedState:
//    qDebug() << "************Call Stop()***********\n";
//    break;
//  case QAudio::IdleState:
//    //传入的数据是空的，音频系统的缓冲区是空的
//    qDebug() << "error: pcm_data is empty:\n";
//    break;
//  case QAudio::InterruptedState:
//    qDebug() << "Higher priority entry\n";
//    dataClear();
//    break;
//  default:
//    break;
//  }
//}
//
// qint64 Player::readData(char *data, qint64 maxlen) {
//  if (len_written >= data_pcm.size())
//    return 0;
//  int len = 0;
//
//  //计算未播放的数据的长度.
//  len = (len_written + maxlen) > data_pcm.size()
//            ? (data_pcm.size() - len_written)
//            : maxlen;
//  memcpy(data, data_pcm.data() + len_written,
//         len);        //把要播放的pcm数据存入声卡缓冲区里.
//  len_written += len; //更新已播放的数据长度.
//  return len;
//}
//
// QAudioFormat format() {
//  QAudioFormat fmt;
//  fmt.setSampleRate(44100);
//  fmt.setChannelCount(2);
//  fmt.setSampleSize(16);
//  fmt.setCodec("audio/pcm");
//  fmt.setByteOrder(QAudioFormat::LittleEndian);
//  fmt.setSampleType(QAudioFormat::SignedInt);
//  QAudioDeviceInfo info(
//      QAudioDeviceInfo::defaultOutputDevice()); //选择默认输出设备
//  if (!info.isFormatSupported(fmt)) {
//    qDebug() << "The output device does not support this format and cannot "
//                "play audio";
//  }
//  return fmt;
//}
//
// QAudioFormat Player::initAudio() {
//  QAudioFormat fmt{};
//  fmt.setSampleRate(44100);
//  fmt.setChannelCount(2);
//  fmt.setSampleSize(16);
//  fmt.setCodec("audio/pcm");
//  fmt.setByteOrder(QAudioFormat::LittleEndian);
//  // QAudioOutput::setVolume设置声音大小之后，声音会出现噪音
//  //解决办法是setSampleType由UnSignedInt改为SignedInt
//  fmt.setSampleType(QAudioFormat::SignedInt);
//  QAudioDeviceInfo info(
//      QAudioDeviceInfo::defaultOutputDevice()); //选择默认输出设备
//  if (!info.isFormatSupported(fmt)) {
//    qDebug() << "The output device does not support this format and cannot "
//                "play audio\n";
//  }
//  return fmt;
//}
