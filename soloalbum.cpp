#include "soloalbum.h"
#include "base.h"
#include "config.h"
#include "ui_soloalbum.h"
#include <QHBoxLayout> //水平布局管理器
SoloAlbum::SoloAlbum(QWidget *parent) : QWidget(parent), ui(new Ui::SoloAlbum) {
  ui->setupUi(this);
  // base = new Base(ui->playlist);
  // base->InitTableWidget();
  Init();
  InitMenu();
  //允许QTableWidget接收QWidget::customContextMenuRequested()信号。
  ui->playlist->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->lab_AlubPic->setScaledContents(true);

  connect(ui->btn_playAll, &QPushButton::clicked, this,
          [&] { emit Alb_playAll(this); });
}

SoloAlbum::~SoloAlbum() { delete ui; }

void SoloAlbum::setDescription(const QString str) {
  ui->lab_Description->setText(str);
}

void SoloAlbum::setPublishTime(const QString str) {
  ui->lab_time->setText(str);
}

void SoloAlbum::setlab_title(const QString str) { ui->lab_title->setText(str); }

void SoloAlbum::setlab_Artist(const QString str) {
  ui->lab_Artist->setText(str);
}

void SoloAlbum::setlab_AlubPic(QPixmap pix) { ui->lab_AlubPic->setPixmap(pix); }

QStringList SoloAlbum::GetPlayList() {
  QStringList playlist{};
  QString url{"https://music.163.com/song/media/outer/url?id="};
  foreach (const auto &rhs, Albtag) {
    playlist.push_back(QString(url + "%1.mp3").arg(rhs.Song_id));
  }
  return playlist;
}

void SoloAlbum::Init() {
  ui->playlist->resize(ui->tabWidget->size());
  ui->tabWidget->tabBar()->setTabText(0, "歌曲列表");
  ui->tabWidget->tabBar()->setTabText(1, "专辑详情");
  QStringList head{"操作", "音乐标题", "歌手", "专辑", "时长"};
  int len = head.length();
  ui->playlist->setColumnCount(len);
  for (int i = 0; i != len; i++) {
    ui->playlist->setHorizontalHeaderItem(i, new QTableWidgetItem(head.at(i)));
  }

  //设置无边框
  ui->playlist->setFrameShape(QFrame::NoFrame);
  //不可编辑
  ui->playlist->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //启动排序
  ui->playlist->setSortingEnabled(false);
  // item 水平表头自适应大小
  ui->playlist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  //设置第一列表头不可拉伸
  ui->playlist->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  ui->playlist->verticalHeader()->setDefaultSectionSize(35);
  //是否使用交替的颜色绘制背景
  ui->playlist->setAlternatingRowColors(true);
  //设置选择行为时每次选择一行
  ui->playlist->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void SoloAlbum::InitMenu() {
  menu = new QMenu(ui->playlist);
  Actplay = new QAction(QIcon(":/images/bottom/btn_play_h.png"), "播放", menu);
  Actnextplay = new QAction(QIcon(":/images/bottom/btn_single_h.png"),
                            "下一首播放", menu);
  Actdownload =
      new QAction(QIcon(":/images/btn_delete_h.png"), "删除(Delete)", menu);

  //添加入右键菜单

  menu->addAction(Actplay);
  menu->addAction(Actnextplay);
  menu->addSeparator();
  menu->addAction(Actdownload);
  menu->setStyleSheet("background-color:white;");
  //播放当前点击的歌曲
  connect(Actplay, &QAction::triggered, this, [&]() {
    int index = ui->playlist->currentRow();
    emit Alb_play(this, Albtag.at(index).Song_id);
  });

  //下一首播放
  connect(Actnextplay, &QAction::triggered, this, [&]() {
    int index = ui->playlist->currentRow();
    QString url =
        QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
            .arg(Albtag.at(index).Song_id);
    emit Alb_Nextplay(this, index, url);
  });
  //下载
  connect(Actdownload, &QAction::triggered, this, [&]() {});
}

void SoloAlbum::setAlbumtag(Albumtag &t) { Albtag.push_back(t); }

void SoloAlbum::LoadData() {
  const int len = Albtag.length();
  ui->playlist->setColumnWidth(0, 100);
  ui->playlist->setColumnWidth(1, 120);
  ui->playlist->setColumnWidth(2, 120);
  ui->playlist->setColumnWidth(3, 120);
  ui->playlist->setColumnWidth(3, 120);

  for (int x = 0; x != len; x++) {
    //插入新的一行
    ui->playlist->insertRow(x);
    QHBoxLayout *hbox = new QHBoxLayout(ui->playlist);
    like = new QPushButton(this);
    like->setIcon(QIcon(":/images/btn_unlike_h.png"));
    down = new QPushButton(this);
    down->setIcon(QIcon(":/images/btn_download_h.png"));
    like->setMaximumSize(QSize(35, 35));
    down->setMaximumSize(QSize(35, 35));
    hbox->addWidget(like);
    hbox->addWidget(down);
    QWidget *widget = new QWidget(this);
    widget->setMaximumSize(80, 35);
    widget->setLayout(hbox);
    ui->playlist->setCellWidget(x, 0, widget);
    QTableWidgetItem *item1 = new QTableWidgetItem(Albtag.at(x).Title);
    QTableWidgetItem *item2 = new QTableWidgetItem(Albtag.at(x).Artist);
    QTableWidgetItem *item3 = new QTableWidgetItem(Albtag.at(x).Album);
    QTableWidgetItem *item4 = new QTableWidgetItem(Albtag.at(x).Duration);
    ui->playlist->setItem(x, 1, item1);
    ui->playlist->setItem(x, 2, item2);
    ui->playlist->setItem(x, 3, item3);
    ui->playlist->setItem(x, 4, item4);
  }

  emit loadOk();
}

QList<int> SoloAlbum::GetPlaylistID() {
  const int n = Albtag.length();
  QList<int> id{};
  foreach (const auto rhs, Albtag) { id.push_back(rhs.Song_id); }
  return id;
}

//鼠标右键
void SoloAlbum::on_playlist_customContextMenuRequested(const QPoint &pos) {
  if (ui->playlist->itemAt(pos) != nullptr) {
    menu->exec(QCursor::pos());
  }
}