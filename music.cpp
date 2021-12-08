#include <qaudio.h>
#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

#include "base.h"
#include <QMimeData>
#include <qevent.h>
#include <qicon.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qtablewidget.h>

#include "./ui_music.h"
#include "local_and_download.h"
#include "lyric.h"
#include "music.h"
#include <QComboBox>
#include <QDebug>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPixmap>
#include <QScrollArea>

#include <QTime>
//随机数
#include <QtGlobal>
using namespace AudioPlayer;

// ffplay -ar 44100 -ac 2 -f s16le -i out.pcm 命令行播放
Music::Music(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::Music), pcmdir(),
	CurrentPlayerListIndex(0) {
	ui->setupUi(this);
	this->setAcceptDrops(true);
	//设置窗体透明
	// this->setAttribute(Qt::WA_TranslucentBackground, true);
	//设置无边框
	this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	//设置默认播放格式--->顺序播放

	init();

	//窗口的关闭丶最大化、最小化的信号槽
	connect(ui->btn_close, &QPushButton::clicked, this, &QMainWindow::close);
	connect(ui->btn_mini, &QPushButton::clicked, this,
		&QMainWindow::showMinimized);

	//关联ListWidget和TableWidget
	connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget,
		&QStackedWidget::setCurrentIndex);

	//双击播放
	connect(localMusic->getTable(), &QTableWidget::cellDoubleClicked, this, [=] {
		CurrentPlayerListIndex = localMusic->getTable()->currentRow();
		qDebug() << "IterDoubleClicked: row = " << CurrentPlayerListIndex;
		Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
		});

	//播放进度条
	connect(Decode, &AudioDeCode::duration, this, &Music::onDuration);
	connect(Decode, &AudioDeCode::seekOk, this, &Music::onSeekOk);
	Mode = PlayMode::Order;
	//自动播放下一首
	connect(Decode, &AudioDeCode::nextsong, this,&Music::PlayerMode);
}

Music::~Music() {
	Decode->stop();
	delete ui;
	ui = nullptr;
}

void Music::init() {
	lyr = new lyric{};
	base = new Base(this);
	player = new Player{};
	Decode = new AudioDeCode{};
	// Decode->setParent(this);
	localMusic = new Local_and_Download{};
	initWidget();
	ui->TopWidget->installEventFilter(this);
	localMusic->getTable()->installEventFilter(this);
	//添加侧边栏按钮
	ui->stackedWidget->insertWidget(0, localMusic);
	//默认音量
	ui->Sli_volum->setMaximum(100);
	ui->Sli_volum->setValue(50);

	QPixmap pixmap;
	pixmap.load(":/images/title_logo.png");
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
	QLabel* lab = new QLabel(tr("我的音乐"), ui->listWidget);
	lab->setStyleSheet(QString("color: rgb(124, 124, 124);"));
	QListWidgetItem* item0 = new QListWidgetItem(ui->listWidget);
	ui->listWidget->setItemWidget(item0, lab);
	QComboBox* box = new QComboBox(this);
	box->hidePopup();
	QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
	QStringList btn{ tr("我创建的歌单"), tr("我喜欢的音乐") };
	box->addItems(btn);

	ui->listWidget->setItemWidget(item, box);
	//点击了其中某一个Item
	connect(ui->listWidget, &QListWidget::itemClicked, this, []() {});
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
void Music::mouseMoveEvent(QMouseEvent* event) {
	if (mMoving) {
		this->move(this->pos() + (event->globalPos() - mLastMousePosition));
		mLastMousePosition = event->globalPos();
	}
}

//鼠标点击事件
void Music::mousePressEvent(QMouseEvent* event) {
	qDebug() << "*****Call mousePressEvent()*********\n";
	//鼠标左键按下
	if (event->button() == Qt::LeftButton) {
		mMoving = true; //移动中
		//记录位置
		mLastMousePosition = event->globalPos();
	}
}

//鼠标松开
void Music::mouseReleaseEvent(QMouseEvent*) {
	qDebug() << "*****Call mouseReleaseEvent()*********\n";
	mMoving = false; //没有移动
}

//快捷键
void Music::Presskey(QKeyEvent* event) {
	//   Ctrl+键盘右键
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
		++CurrVolume;
		//        MyPlay->Playlist->previous();
		//        MyPlay->Play->play();
		break;
	case Qt::Key_Right:
		//        MyPlay->Playlist->next();
		//        MyPlay->Play->play();
		break;
	case Qt::Key_O:

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
bool Music::eventFilter(QObject* obj, QEvent* event) {
	if (obj == ui->TopWidget) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			Presskey(keyEvent);
			return true; //该事件已被处理
		}
		else {
			return false; //其他事件
		}
	}

	// ctrl + → 播放下一首歌曲
	if (obj == localMusic->getTable()) {
		//键盘按下事件
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* evn = static_cast<QKeyEvent*>(event);
			if (evn->key() == Qt::Key_Right) {
				++CurrentPlayerListIndex;
				if (CurrentPlayerListIndex > localMusic->PlayerList().length()) {
#if DEBUG
					qDebug() << tr("超出列表长度，将 CurrentPlayerListIndex 重置为0\n");
#endif
					CurrentPlayerListIndex = 0;
				}
				else {
					// play(CurrentPlayerListIndex);
					Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
					setBottomInformation(Decode->GetTag());
				}
			}
		}
	}
	// ctrl + ← 播放上一首歌曲
	if (obj == localMusic->getTable()) {
		//键盘按下事件
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* evn = static_cast<QKeyEvent*>(event);
			if (evn->key() == Qt::Key_Left) {
				--CurrentPlayerListIndex;
				if (CurrentPlayerListIndex == 0) {
#if DEBUG
					qDebug() << tr("已经在列表首位，重置到末尾\n");
#endif
					CurrentPlayerListIndex = localMusic->PlayerList().length();
				}
				else {
					Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
					setBottomInformation(Decode->GetTag());
				}
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
}

void Music::closeEvent(QCloseEvent* event) {
	int rect = QMessageBox::question(this, tr("Tip"), tr("确认关闭该软件吗?"));
	if (rect == QMessageBox::Yes) {
		event->accept(); //确认关闭
	}
	else {
		event->ignore(); //忽略此事件
	}
}

void Music::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasUrls()) //判断拖的类型
	{
		event->acceptProposedAction();
	}
	else {
		event->ignore();
	}
}

void Music::dropEvent(QDropEvent* event) {
	if (event->mimeData()->hasUrls()) //判断放的类型
	{
		QList<QUrl> List = event->mimeData()->urls();

		if (List.length() != 0) {
			// ui->line_audioPath->setText(mp3);
		}
	}
	else {
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

	QString currentTime =
		QString("%1:%2")
		.arg((currentMs1 / 1000000) / 60, 2, 10, QChar('0'))
		.arg((currentMs1 / 1000000) % 60, 2, 10, QChar('0'));
	// QString destTime = QString("%1:%2:%3")
	//                        .arg(destMs1 / 360000 % 60, 2, 10, QChar('0'))
	//                        .arg(destMs1 / 6000 % 60, 2, 10, QChar('0'))
	//                        .arg(destMs1 / 1000 % 60, 2, 10, QChar('0'));

	ui->lab_time->setText(currentTime);

	if (!sliderSeeking) //未滑动
	{
		ui->playslider->setMaximum(destMs);
		ui->playslider->setValue(currentMs);
	}
}

void Music::on_btn_pictrue_clicked() {
	// 歌词界面未打开
	if (lyr->isHidden()) {
		ui->listWidget->hide();
		ui->stackedWidget->hide();
		ui->horizontalLayout_11->addWidget(lyr);
		lyr->show();
	}
	else {
		lyr->close();
		ui->listWidget->show();
		ui->stackedWidget->show();
	}
}

//滑动滑块
void Music::on_Sli_volum_valueChanged(int value) {
	//设置播放音量
	if (Decode->audio) {
		Decode->audio->setVolume(value / qreal(100.0));
	}
	ui->btn_volum->setStyleSheet(
		"#btn_volum{border-image:url(:/images/Bottom/btn_volume_open.png)}");
}

void Music::on_btn_volum_clicked() {
	if (ui->Sli_volum->value() == STATE_VOLUME) {
		//恢复音量大小
		ui->Sli_volum->setValue(CurrVolume);
		ui->btn_volum->setStyleSheet(
			"#btn_volum{border-image:url(:/images/Bottom/btn_volume_open.png)}");
	}
	else {
		CurrVolume = ui->Sli_volum->value(); //先记录当前音量
		ui->Sli_volum->setValue(0);
		ui->btn_volum->setStyleSheet(
			"#btn_volum{border-image:url(:/images/Bottom/btn_volume_close.png)}");
	}
}

//检测当前播放状态
void Music::on_btn_stop_clicked() {
	switch (state) {
	case State::resume: //恢复播放
		state = State::pause;
		Decode->resume();
		ui->btn_stop->setStyleSheet(
			"border-image:url(:/images/Bottom/btn_stop_h.png)");
		break;
	case State::pause: //暂停
		state = State::resume;
		Decode->pause();
		ui->btn_stop->setStyleSheet(
			"border-image:url(:/images/Bottom/btn_stop_n.png)");
		break;
	default:
		state = State::pause;
		sliderSeeking = false;
		CurrentPlayerListIndex = 0;
		Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
		setBottomInformation(Decode->GetTag());
		break;
	}
}

void Music::setBottomInformation(Mp3tag* tag) {
	QPixmap pixmap = QPixmap::fromImage(tag->Picture);
	int width = ui->btn_pictrue->width();
	int height = ui->btn_pictrue->height();
	ui->btn_pictrue->setIconSize(QSize(width + 5, height));
	ui->btn_pictrue->setIcon(QIcon(pixmap));
	ui->btn_pictrue->setStyleSheet("");
	ui->lab_message->setText(QString("%1\n%2").arg(tag->Artis).arg(tag->Title));
	ui->lab_time->setText(tag->Duration);
}

//上一首
void Music::Previous()
{
	if (CurrentPlayerListIndex == 0) {
		CurrentPlayerListIndex = localMusic->PlayerList().length() - 1;
	}
	else {
		--CurrentPlayerListIndex;
	}
	Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
	setBottomInformation(Decode->GetTag());
}

//下一首
void Music::Next()
{
	++CurrentPlayerListIndex;
	if (CurrentPlayerListIndex >= localMusic->PlayerList().length()) {
		CurrentPlayerListIndex = 0;
	}
	Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
	setBottomInformation(Decode->GetTag());
}

void Music::on_btn_prev_clicked() { Previous(); }

void Music::on_btn_next_clicked() {
	++CurrentPlayerListIndex;
	if (CurrentPlayerListIndex >= localMusic->PlayerList().length()) {
		CurrentPlayerListIndex = 0;
	}
	Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
	setBottomInformation(Decode->GetTag());
}

//最大化与还原
void Music::on_btn_max_clicked() {
	qDebug() << "最大化，还原\n";
	static bool max = false;
	static QRect location = this->geometry();
	if (max) {
		this->setGeometry(location); //回复窗口原大小和位置
		ui->btn_max->setIcon(QIcon(":/MAX_.png"));
	}
	else {
		location = this->geometry(); //最大化前记录窗口大小和位置
		ui->btn_max->setIcon(QIcon(":/minMAX.png"));
		this->showFullScreen(); //设置窗口铺满全屏
	}
	max = !max;
}

void Music::on_btn_login_clicked() {}

//进度条按压处理
void Music::on_playslider_sliderPressed() { sliderSeeking = true; }

//进度条释放处理
void Music::on_playslider_sliderReleased() {
	int value = ui->playslider->value();
	Decode->seek(value);
}

void Music::on_btn_mode_clicked() {
	//static int Mode = 0;
	//switch (Mode) {
	//case static_cast<int>(PlayMode::Order):
	//	//设置单曲循环
	//	Mode = static_cast<int>(PlayMode::Single);
	//	ui->btn_mode->setText(tr("Single"));
	//	break;

	//case static_cast<int>(PlayMode::Single):
	//	//设置随机播放
	//	Mode = static_cast<int>(PlayMode::Random);
	//	ui->btn_mode->setText(tr("Random"));
	//	break;

	//case static_cast<int>(PlayMode::Random):
	//	//设置顺序播放
	//	Mode = static_cast<int>(PlayMode::Order);
	//	ui->btn_mode->setText(tr("Order"));
	//	break;
	//}
	setPlayMode(Mode);
}

void Music::setPlayMode(PlayMode &Mode)
{
	switch (Mode)
	{
	case Music::PlayMode::Order:
		Mode =PlayMode::Single;
		ui->btn_mode->setStyleSheet("");
		break;
	case Music::PlayMode::Single:
		Mode = PlayMode::Random;
		ui->btn_mode->setStyleSheet("");
		break;
	case Music::PlayMode::Random:
		Mode = PlayMode::Order;
		ui->btn_mode->setStyleSheet("");
		break;
	default:
		break;
	}
}

void Music::PlayerMode() {
	//static int Mode = 0;
	//if (Mode == static_cast<int>(PlayMode::Order)) {
	//	Next();
	//}
	//else if (Mode == static_cast<int>(PlayMode::Single)) {
	//	qDebug() << "Single***********\n"; return;
	//	//Decode->play()
	//}
	//else if (Mode == static_cast<int>(PlayMode::Random)) {
	//	//设置随机种子，即随机数在种子值到32767之间
	//	qsrand(QTime::currentTime().msec());
	//	//随机生成0到播放列表的长度的随机数
	//	CurrentPlayerListIndex = qrand() % localMusic->PlayerList().length();
	//}
	//else {
	//	++Mode;
	//}
	
	switch (Mode) {
	//顺序播放
	case PlayMode::Order:
		++CurrentPlayerListIndex;
		if (CurrentPlayerListIndex >= localMusic->PlayerList().length()) {
			CurrentPlayerListIndex = 0;
		}
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
	Decode->play(localMusic->PlayerList().at(CurrentPlayerListIndex));
	setBottomInformation(Decode->GetTag());
}


