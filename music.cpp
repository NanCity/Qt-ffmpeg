#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "music.h"
#include "./ui_music.h"
#include "local_and_download.h"
#include "login.h"
#include "lyric.h"
#include "networkutil.h"
#include "personform.h"
#include "search.h"
#include "skin.h"
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QIcon>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QScrollArea>
#include <QTableWidget>
#include <QTime>
//随机数
#include <QtGlobal>

// ffplay -ar 44100 -ac 2 -f s16le -i out.pcm 命令行播放
Music::Music(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Music), CurrentPlayerListIndex(0) {
  ui->setupUi(this);
  this->setAcceptDrops(true);
  //设置窗体透明
  // this->setAttribute(Qt::WA_TranslucentBackground, true);
  //设置无边框
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  init();

  //窗口的关闭丶最大化、最小化的信号槽
  connect(ui->btn_close, &QPushButton::clicked, this, &QMainWindow::close);
  connect(ui->btn_mini, &QPushButton::clicked, this,
          &QMainWindow::showMinimized);

  //关联ListWidget和TableWidget
  connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget,
          &QStackedWidget::setCurrentIndex);
  connect(localMusic, &Local_and_Download::t_loaded, this,&Music::on_setPlatList);

  //双击播放
  connect(localMusic->getTable(), &QTableWidget::cellDoubleClicked, this, [=] {
    CurrentPlayerListIndex = localMusic->getTable()->currentRow();
    ui->btn_stop->setStyleSheet(
        "border-image:url(:/images/bottom/btn_pause_h.png)");
    Decode->play(playlist.at(CurrentPlayerListIndex));
    SetBottonInformation(Decode->GetTag());
  });

  //播放进度条
  connect(Decode, &AudioDeCode::duration, this, &Music::onDuration);
  connect(Decode, &AudioDeCode::seekOk, this, &Music::onSeekOk);
  Mode = PlayMode::Order;
  //自动播放下一首
  connect(Decode, &AudioDeCode::nextsong, this, &Music::PlayerMode);

  //接收鼠标右键菜单的信号
  connect(localMusic, &Local_and_Download::t_play, this,
          [this](const int index) {
            //Decode->play(localMusic->PlayerList().at(index));
            Decode->play(playlist.at(index));
            SetBottonInformation(Decode->GetTag());
          });

  //播放下一首
  connect(localMusic, &Local_and_Download::t_nextplay, this,
          [=](const int index) {
            if (Decode->isFinished()) {
              Decode->play(playlist.at(index));
              SetBottonInformation(Decode->GetTag());
            }
          });

  //更换主题
  connect(Skin, &skin::setThem, this, [=](QByteArray rhs) {
    // qApp->setStyleSheet(rhs);
  });

  //删除播放列表
  connect(localMusic->getTable(), &QTableWidget::removeRow, this,
          [&](int row) { --CurrentPlayerListIndex; });

  //播放搜索到的歌曲
  connect(search, &Search::play, this, &Music::on_playSearchMusic);
}

Music::~Music() {
  Decode->stop();
  delete ui;
  ui = nullptr;
}

void Music::init() {
  lyr = new lyric{};
  Decode = new AudioDeCode{};
  localMusic = new Local_and_Download{};
  Skin = new skin(this);
  login = new Login(this);
  perform = new PersonForm(this);
  search = new Search(this);
  search->hide();
  initWidget();
  ui->TopWidget->installEventFilter(this);
  localMusic->getTable()->installEventFilter(this);
  //添加侧边栏按钮
  ui->stackedWidget->insertWidget(0, localMusic);
  //默认音量
  ui->Sli_volum->setMaximum(100);
  ui->Sli_volum->setValue(50);

  QPixmap pixmap;
  pixmap.load(":/images/top/logo.png");
  //    ui->btn_lyric->setStyleSheet("QPushButton#btn_lyric:!hover{background-color:rgb(103,
  //    103, 230)};");
  ui->title_logo->setPixmap(pixmap);
  //设置无窗口模式
  setWindowFlags(Qt::FramelessWindowHint);
  ui->playslider->setMinimum(0);
  ui->playslider->setMaximum(100);
  //悬停提示
  HoverTip();
}

void Music::initWidget() {
  ui->listWidget->addItem(QString(QObject::tr("本地与下载")));
  ui->listWidget->addItem(QString(QObject::tr("我的收藏")));
  QLabel *lab = new QLabel(tr("我的音乐"), ui->listWidget);
  lab->setStyleSheet(QString("color: rgb(124, 124, 124);"));
  QListWidgetItem *item0 = new QListWidgetItem(ui->listWidget);
  ui->listWidget->setItemWidget(item0, lab);
  QComboBox *box = new QComboBox(this);
  box->hidePopup();
  QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
  QStringList btn{tr("我创建的歌单"), tr("我喜欢的音乐")};
  box->addItems(btn);

  ui->listWidget->setItemWidget(item, box);
  //点击了其中某一个Item
  connect(ui->listWidget, &QListWidget::itemClicked, this, [&]() {
    search->hide();
    ui->stackedWidget->show();
  });
  //  connect(box,&QComboBox::activated,ui->listWidget ,[this](){/*
  //  项被点击时发出该信号 */});
  box->setStyleSheet(
      "#box{border: 1px solid gray;"
      " subcontrol-origin: padding;"
      " subcontrol-position: top right;"
      " width: 20px;"
      " border-left-width: 1px;"
      " border-left-color: darkgray;"
      " border-left-style: solid; /* just a single line */"
      " border-top-right-radius: 3px; /* same radius as the QComboBox */"
      " border-bottom-right-radius: 3px;}");
}

void Music::initTableWidget() {}

// mMoving ： bool值，判断鼠标是否移动
// mLastMousePosition： QPoint类型，记录以前的位置
void Music::mouseMoveEvent(QMouseEvent *event) {
  if (mMoving) {
    this->move(this->pos() + (event->globalPos() - mLastMousePosition));
    mLastMousePosition = event->globalPos();
  }
}

//鼠标点击事件
void Music::mousePressEvent(QMouseEvent *event) {
  //鼠标左键按下
  if (event->button() == Qt::LeftButton) {
    mMoving = true; //移动中
    //记录位置
    mLastMousePosition = event->globalPos();
  }
}

//鼠标松开
void Music::mouseReleaseEvent(QMouseEvent *) {
  mMoving = false; //没有移动
}

//快捷键
void Music::Presskey(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_M:
    setWindowState(Qt::WindowMaximized);
    break;
  case Qt::Key_N:
    setWindowState(Qt::WindowMinimized);
    break;
  case Qt::Key_Q:
    close();
    break;
  case Qt::Key_Left:
    Previous(localMusic->PlayerList());
    break;
  case Qt::Key_Right:
    Next(localMusic->PlayerList());
    break;
  case Qt::Key_Up:
    ui->Sli_volum->setValue(++CurrVolume);
    break;
  case Qt::Key_Down:
    ui->Sli_volum->setValue(--CurrVolume);
    break;
  default:
    event->key();
    break;
  }
}

//事件过滤器
bool Music::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->TopWidget) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      Presskey(keyEvent);
      return true; //该事件已被处理
    } else {
      return false; //其他事件
    }
  }

  // ctrl + → 播放下一首歌曲
  if (obj == localMusic->getTable()) {
    //键盘按下事件
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *evn = static_cast<QKeyEvent *>(event);
      if (evn->key() == Qt::Key_Right) {
        //更好的做法是发送一个信号，可以获得更多兼容性
        Next(localMusic->PlayerList());
      }
    }
  }
  // ctrl + ← 播放上一首歌曲
  if (obj == localMusic->getTable()) {
    //键盘按下事件
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *evn = static_cast<QKeyEvent *>(event);
      if (evn->key() == Qt::Key_Left) {
        Previous(localMusic->PlayerList());
      }
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

void Music::closeEvent(QCloseEvent *event) {
  int rect = QMessageBox::question(this, tr("Tip"), tr("确认关闭该软件吗?"));
  if (rect == QMessageBox::Yes) {
    event->accept(); //确认关闭
  } else {
    event->ignore(); //忽略此事件
  }
}

void Music::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) //判断拖的类型
  {
    event->acceptProposedAction();
  } else {
    event->ignore();
  }
}

void Music::dropEvent(QDropEvent *event) {
  if (event->mimeData()->hasUrls()) //判断放的类型
  {
    QList<QUrl> List = event->mimeData()->urls();

    if (List.length() != 0) {
      // ui->line_audioPath->setText(mp3);
    }
  } else {
    event->ignore();
  }
}

void Music::HoverTip() {
  ui->btn_mini->setToolTip(tr("最小化"));
  ui->btn_max->setToolTip(tr("最大化"));
  ui->btn_close->setToolTip(tr("关闭"));
  ui->btn_skin->setToolTip(tr("皮肤"));
  ui->btn_setting->setToolTip(tr("设置"));
  ui->btn_next->setToolTip(tr("下一首(ctrl + →)"));
  ui->btn_prev->setToolTip(tr("上一首(ctrl + ←)"));
  ui->btn_stop->setToolTip(tr("暂停(ctrl + space)"));
  ui->btn_login->setToolTip(tr("登录"));
  ui->btn_lyric->setToolTip(tr("歌词"));
  ui->btn_volum->setToolTip(tr("音量"));
}

void Music::onSeekOk() { sliderSeeking = false; }

void Music::onDuration(int currentMs, int destMs) {
  static int currentMs1 = -1, destMs1 = -1;
  if (currentMs1 == currentMs && destMs1 == destMs) {
    return;
  }

  currentMs1 = currentMs;
  destMs1 = destMs;
  // qDebug() << "onDuration：" << currentMs << destMs << sliderSeeking;
  //计算秒
  // static int  s = currentMs1 / 1000 % 60;
  QString currentTime = QString("%1 : %2")
                            .arg(currentMs1 / 1000 / 60 % 60, 2, 10, QChar('0'))
                            .arg(currentMs1 / 1000 % 60, 2, 10, QChar('0'));
  ui->lab_duration->setText(currentTime);
  if (!sliderSeeking) //未滑动
  {
    ui->playslider->setMaximum(destMs);
    ui->playslider->setValue(currentMs);
  }
}

void Music::on_setPlatList(QStringList list) { playlist = list; }

//滑动滑块
void Music::on_Sli_volum_valueChanged(int value) {
  //设置播放音量
  if (Decode->audio) {
    Decode->audio->setVolume(value / qreal(100.0));
  }
  if (ui->Sli_volum->value() == 0) {
    ui->btn_volum->setStyleSheet(
        "#btn_volum{border-image:url(:/images/bottom/btn_mute.png)}");
  } else {
    ui->btn_volum->setStyleSheet(
        "#btn_volum{border-image:url(:/images/bottom/btn_volume.png)}");
  }
}

void Music::on_btn_volum_clicked() {
  if (ui->Sli_volum->value() == STATE_VOLUME) {
    //恢复音量大小
    ui->Sli_volum->setValue(CurrVolume);
    ui->btn_volum->setStyleSheet(
        "#btn_volum{border-image:url(:/images/bottom/btn_volume.png)}");
  } else {
    CurrVolume = ui->Sli_volum->value(); //先记录当前音量
    ui->Sli_volum->setValue(0);
    ui->btn_volum->setStyleSheet(
        "#btn_volum{border-image:url(:/images/bottom/btn_mute.png)}");
  }
}

//检测当前播放状态
void Music::on_btn_stop_clicked() {

  switch (state) {
  case State::resume: //恢复播放
    state = State::pause;
    Decode->resume();
    ui->btn_stop->setStyleSheet(
        "border-image:url(:/images/bottom/btn_pause_h.png)");
    break;
  case State::pause: //暂停
    state = State::resume;
    Decode->pause();
    ui->btn_stop->setStyleSheet(
        "border-image:url(:/images/bottom/btn_play_h.png)");
    break;
  default:
    state = State::pause;
    sliderSeeking = false;
    if (localMusic->PlayerList().isEmpty()) {
      qDebug() << "PlayList isempty...\n\n";
      return;
    }
    CurrentPlayerListIndex = 0;
    Decode->play(playlist.at(CurrentPlayerListIndex));
    SetBottonInformation(Decode->GetTag());
    ui->btn_stop->setStyleSheet(
        "border-image:url(:/images/bottom/btn_pause_h.png)");
    break;
  }
}

void Music::SetBottonInformation(Mp3tag *tag) {
  QPixmap pixmap = QPixmap::fromImage(tag->Picture);
  pixmap.scaled(ui->btn_pictrue->size(), Qt::KeepAspectRatio);
  //自适应缩放图片
  ui->btn_pictrue->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);
  int width = ui->btn_pictrue->width();
  int height = ui->btn_pictrue->height();
  ui->btn_pictrue->setIconSize(QSize(width + 5, height));
  ui->btn_pictrue->setIcon(QIcon(pixmap));
  ui->btn_pictrue->setStyleSheet("");
  ui->lab_message->setText(QString("%1\n%2").arg(tag->Artis).arg(tag->Title));
  ui->lab_time->setText(tag->Duration);
  lyr->setMessage(tag->Picture, tag->Artis + QString("--%0").arg(tag->Ablue),
                  tag->Title);
}

void Music::setBottomInformation() {
  int n = search->CurInex();
  QString artist = search->getSearchResults().at(n).singer_name;
  QString title = search->getSearchResults().at(n).song_name;
  QPixmap map = search->getAlbumArt();
  //自适应缩放图片
  ui->btn_pictrue->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);
  ui->btn_pictrue->setIcon(map);
  ui->btn_pictrue->setIconSize(ui->btn_pictrue->size());
  ui->lab_message->setText(QString("%1\n%2").arg(artist).arg(title));
  lyr->setMessage(map.toImage(), artist, title);
}

//上一首
void Music::Previous(QStringList &playerlist) {
  if (!playerlist.isEmpty()) {
    --CurrentPlayerListIndex;
    if (CurrentPlayerListIndex == -1) {
      CurrentPlayerListIndex = playerlist.length() - 1;
    }
    Decode->play(playlist.at(CurrentPlayerListIndex));
    SetBottonInformation(Decode->GetTag());
  } else {
    QMessageBox::information(this, tr("Error"), tr("播放列表为空!!!!"),
                             QMessageBox::Yes);
  }
}
//下一首
void Music::Next(QStringList &playerlist) {
  if (!playerlist.isEmpty()) {
    ++CurrentPlayerListIndex;
    if (CurrentPlayerListIndex == playerlist.length()) {
      CurrentPlayerListIndex = 0;
    }
    Decode->play(playlist.at(CurrentPlayerListIndex));
    SetBottonInformation(Decode->GetTag());
  } else {
    QMessageBox::information(this, tr("Error"), tr("播放列表为空!!!!"),
                             QMessageBox::Yes);
  }
}

void Music::on_btn_prev_clicked() { Previous(localMusic->PlayerList()); }

void Music::on_btn_next_clicked() {
  if (localMusic->PlayerList().isEmpty()) {
    qDebug() << "播放列表为空\n";
    return;
  }
  ++CurrentPlayerListIndex;
  if (CurrentPlayerListIndex >= localMusic->PlayerList().length()) {
    CurrentPlayerListIndex = 0;
  }
  Decode->play(playlist.at(CurrentPlayerListIndex));
  SetBottonInformation(Decode->GetTag());
}

//最大化与还原
void Music::on_btn_max_clicked() {
  static bool max = false;
  static QRect location = this->geometry();
  if (max) {
    this->setGeometry(location); //回复窗口原大小和位置
    ui->btn_max->setIcon(QIcon(":/MAX_.png"));
    max = false;
  } else {

    location = this->geometry(); //最大化前记录窗口大小和位置
    ui->btn_max->setIcon(QIcon(":/minMAX.png"));
    // this->showFullScreen(); //设置窗口铺满全屏
    this->showMaximized();
    max = true;
  }
}

//登陆
void Music::on_btn_login_clicked() { login->show(); }

//进度条按压处理
void Music::on_playslider_sliderPressed() { sliderSeeking = true; }

//进度条释放处理
void Music::on_playslider_sliderReleased() {
  int value = ui->playslider->value();
  Decode->seek(value);
}

void Music::on_btn_mode_clicked() { setPlayMode(Mode); }

void Music::setPlayMode(PlayMode &Mode) {
  switch (Mode) {
  case Music::PlayMode::Order:
    Mode = PlayMode::Single;
    ui->btn_mode->setStyleSheet(
        "border-image:url(:/images/bottom/btn_single_h.png)");
    break;
  case Music::PlayMode::Single:
    Mode = PlayMode::Random;
    ui->btn_mode->setStyleSheet(
        "border-image:url(:/images/bottom/btn_random_h.png)");
    break;
  case Music::PlayMode::Random:
    Mode = PlayMode::Order;
    ui->btn_mode->setStyleSheet(
        "border-image:url(:/images/bottom/btn_order_h.png)");
    break;
  default:
    break;
  }
}

void Music::PlayerMode() {
  switch (Mode) {
    //顺序播放
  case PlayMode::Order:
    Next(localMusic->PlayerList());
    break;
    //单曲循环
  case PlayMode::Single:
    break;
  case PlayMode::Random:
    //设置随机种子，即随机数在种子值到32767之间
    qsrand(QTime::currentTime().msec());
    //随机生成0到播放列表的长度的随机数
    CurrentPlayerListIndex = qrand() % localMusic->PlayerList().length();
    break;
  }
  Decode->play(playlist.at(CurrentPlayerListIndex));
  SetBottonInformation(Decode->GetTag());
}

void Music::on_btn_pictrue_clicked() {
  // 歌词界面未打开
  if (lyr->isHidden()) {
    ui->listWidget->hide();
    ui->stackedWidget->hide();
    ui->horizontalLayout_11->addWidget(lyr);

    search->close();
    lyr->show();
  } else {
    lyr->close();
    ui->listWidget->show();
    ui->stackedWidget->show();
  }
}
void Music::on_btn_skin_clicked() {
  if (Skin->isHidden()) {
    /*
     * 获取当前鼠标的位置，
     * 将控件设置到指定的位置，并设置大小
     */
    QPoint skin_xy = QCursor::pos();
    Skin->setGeometry(skin_xy.x() - 100, skin_xy.y() + 30, 320, 320);
    Skin->show();
  } else {
    Skin->hide();
  }
}

void Music::on_btn_personmessage_clicked() {
  if (perform->isHidden()) {
    QPoint skin_xy = QCursor::pos();
    perform->setGeometry(skin_xy.x() - 80, skin_xy.y() + 50, 262, 297);
    perform->show();
  } else {
    perform->hide();
  }
}

void Music::on_lineEdit_search_returnPressed() {
  search->GetSearchText(ui->lineEdit_search->text());
  if (search->isHidden()) {
    ui->stackedWidget->hide();
    ui->horizontalLayout_11->addWidget(search);
    search->show();
  }
}

void Music::on_playSearchMusic(const int songid) {
  mutex.lock();
  setBottomInformation();
  mutex.unlock();

  playlist.clear();
  playlist = search->GetPlaylistID();

  QString url = QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
                    .arg(songid);
  Decode->play(url);


}
