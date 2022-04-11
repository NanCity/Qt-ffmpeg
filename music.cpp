#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "music.h"
#include "./ui_music.h"
#include "singerdetails.h"
#include "base.h"
#include "songmenu.h"
#include "cloudmusic.h"
#include "config.h"
#include "desktoplyrics.h"
#include "discovrmusic.h"
#include "local_and_download.h"
#include "login.h"
#include "singerdetails.h"
#include "lyric.h"
#include "networkutil.h"
#include "personform.h"
#include "recommendeddaily.h"
#include "RecommendPlaylist/recommendplaylist.h"
#include "search.h"
#include "skin.h"
#include "soloalbum.h"
#include "tag.h"
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
#include <photowall/photowall.h>
// ffplay -ar 44100 -ac 2 -f s16le -i out.pcm 命令行播放
Music::Music(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::Music), CurrentPlayerListIndex(0) {
	ui->setupUi(this);
	qDebug() << "*********************QSslSocket=" << QSslSocket::sslLibraryBuildVersionString();
	//setAttribute(Qt::WA_DeleteOnClose);
	init();
	Mode = PlayMode::Order;
	state = State::none;
	//设置窗体透明
	// this->setAttribute(Qt::WA_TranslucentBackground, true);
	//  设置无边框
	this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
		Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

	//安装事件过滤器
	InstallEventFilter();
	//窗口的关闭丶最大化、最小化的信号槽
	connect(ui->btn_close, &QPushButton::clicked, this, &QMainWindow::close);
	connect(ui->btn_mini, &QPushButton::clicked, this,
		&QMainWindow::showMinimized);

	//关联侧边栏到statckedWidget
	connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget,
		&QStackedWidget::setCurrentIndex);

	connect(ui->down_listWidget, &QListWidget::currentRowChanged,
		ui->stackedWidget,
		[&](int cur) { ui->stackedWidget->setCurrentIndex(cur + 6); });

	connect(ui->down_listWidget, &QListWidget::itemClicked, this,
		&Music::on_down_listWidgetClicked);

	connect(login, &Login::LoginSucces, this, &Music::on_login_succes);
	/* connect(ui->SongMenuList, &QListWidget::currentRowChanged,
	 ui->stackedWidget, &QStackedWidget::setCurrentIndex);

	 connect(ui->CollectSongMenuList, &QListWidget::currentRowChanged,
			 ui->stackedWidget, &QStackedWidget::setCurrentIndex);*/
	connect(this, &Music::updateSongLrc, this,
		[&](int sec) { lyr->showcontent(sec); });
	//图片自适应大小
	ui->btn_usepic->setScaledContents(true);
	//各个界面的connect
	AlbConnect();
	DecodeConnect();
	SearchConnect();
	SocalMusicConnect();
	CloudMusicConnect();
	LocalMusicConnect();
	PersonFormConnect();
	RecommendedDailyConnect();
	SingerDetailsConnect();
	SongMenuConnect();
	//默认打开首页
	ui->stackedWidget->setCurrentIndex(0);
}

Music::~Music() {
	delete ui;
	ui = nullptr;
	delete lyr;
	lyr = nullptr;
	delete config;
	config = nullptr;
	Decode->stop();
	//关闭 node.exe 进程 
	const char* cmd{ "taskkill /f /t /im node.exe" };
	system(cmd);
	//getchar();
	qDebug() << "Music 析构完毕";
}

void Music::on_login_succes() {
	//设置账号名
	ui->btn_login->setText(config->GetValue("/Userinfo/nickname"));
	QPixmap pix("../Userpix/user.png");
	/*pix = pix.scaled(ui->btn_usepic->size(), Qt::KeepAspectRatio,
					 Qt::SmoothTransformation);*/

	ui->btn_usepic->setPixmap(pix);
	QString dynamic = config->GetValue("/Userinfo/dynamic");
	perform->setdynamic_nub(dynamic);

	QString fans = config->GetValue("/Userinfo/followeds");
	perform->setlab_fans_nub(fans);

	QString follow = config->GetValue("/Userinfo/follows");
	perform->setlab_follow_nub(follow);

	QString lev = config->GetValue("/Userinfo/level");
	perform->setlab_level(QString("Lv.%1").arg(lev));
	QString vip = config->GetValue("/Userinfo/vip");
	if (vip == "No VIP") {
		ui->btn_personmessage->setStyleSheet("border-image:url{:/images/s.png}");
		return;
	}
	else {
		perform->setlab_vip(vip);
		ui->btn_personmessage->setStyleSheet("border-image:url(:/images/vip.png)");
	}
}

void Music::AlbConnect() {
	connect(DicMusic->getSoloAlbumUi(), &SoloAlbum::loadOk,
		[&]() { ui->stackedWidget->setCurrentIndex(11); });

	connect(DicMusic->getSoloAlbumUi(), &SoloAlbum::Alb_playAll, this,
		[&](auto rhs) { On_NetplayAll(rhs); });

	//播放搜索到的歌曲（播放）
	connect(DicMusic->getSoloAlbumUi(), &SoloAlbum::Alb_play, this,
		[&](auto rhs, int index) { On_Netplay(rhs, index); });

	//播放搜索到的歌曲（下一首播放）
	connect(DicMusic->getSoloAlbumUi(), &SoloAlbum::Alb_Nextplay, this,
		[&](auto rhs, int index, const QString _id) {
			On_NetNextPlay(rhs, index, _id);
		});
}

void Music::DecodeConnect() {
	//播放进度条
	connect(Decode, &AudioDeCode::duration, this, &Music::onDuration);
	connect(Decode, &AudioDeCode::seekOk, this, &Music::onSeekOk);

	//自动播放下一首
	connect(Decode, &AudioDeCode::nextsong, this, &Music::PlayerMode);

	//自动跟新底部标签和专辑封面等信息
	connect(Decode->tag, &M_Tag::parseOk, this,
		[&]() { SetBottonInformation(Decode->tag); });

	connect(Decode, &AudioDeCode::LocalparseOk, this,
		[&]() { SetBottonInformation(Decode->tag); });
}

void Music::SearchConnect() {
	//播放搜索到的歌曲（播放全部）
	connect(search, &Search::playAll, this,
		[&](auto rhs) { this->On_NetplayAll(rhs); });
	//播放当前点击的歌曲
	connect(search, &Search::play, this,
		[&](auto rhs, int index) { On_Netplay(rhs, index); });

	//播放搜索到的歌曲（下一首播放）
	connect(search, &Search::Nextplay, this,
		[&](auto rhs, int index, QString url) {
			On_NetNextPlay(rhs, index, url);
		});
	//点击了top榜单
	connect(search, &Search::clickedTopSeach, this, [&]() {
		ui->stackedWidget->setCurrentIndex(14);
		search->show();
		});

	//歌手详情
	//connect(search, &Search::loadSinger, this, [&](Singer singer) {
	//	ui->stackedWidget->setCurrentIndex(13);
	//	singetdeatils->setmesg(singer);
	//	singetdeatils->show();
	//	});
	connect(search, &Search::getid, singetdeatils, &SingerDetails::setID);
}

void Music::SocalMusicConnect() {}

void Music::CloudMusicConnect() {
	/* connect(cloudMusic, &CloudMusic::play, this,
			  [&]( auto rhs, const int id) { On_Netplay(rhs, id); });*/
}

void Music::LocalMusicConnect() {
	//设置播放列表(从本地加载)
	connect(localMusic, &Local_and_Download::t_loaded, this,
		&Music::on_setPlatList);
	//双击播放
	connect(localMusic->getTable(), &QTableWidget::cellDoubleClicked, this, [=] {
		CurrentPlayerListIndex = localMusic->getTable()->currentRow() - 1;
		ui->btn_stop->setStyleSheet(
			"border-image:url(:/images/bottom/btn_pause_h.png)");
		PlayerMode();
		});

	connect(localMusic, &Local_and_Download::t_play, this,
		[this](const int index) {
			CurrentPlayerListIndex = index;
			Decode->DeCodeTag(
				(playlist.at(CurrentPlayerListIndex).toStdString().c_str()));
			// SetBottonInformation(GetTag(tag));
			Decode->play(playlist.at(CurrentPlayerListIndex));
		});

	//播放下一首
	connect(localMusic, &Local_and_Download::t_nextplay, this,
		[=](const int index) {
			if (Decode->isFinished()) {
				Decode->play(playlist.at(index));
				Decode->DeCodeTag((playlist.at(index).toStdString().c_str()));
				// SetBottonInformation(GetTag(tag));
			}
		});

	//删除播放列表
	connect(localMusic->getTable(), &QTableWidget::removeRow, this, [&](int row) {
		playlist.removeAt(row);
		--CurrentPlayerListIndex;
		});
}

void Music::PersonFormConnect() {
	connect(perform, &PersonForm::out, [&]() {
		ui->btn_login->setText("未登录");
		ui->btn_usepic->setPixmap(QPixmap(""));
		});

	connect(perform, &PersonForm::CheckinOk, this,
		[]() { fprintf(stdout, "签到成功"); });
}

void Music::RecommendedDailyConnect() {
	connect(DicMusic->getRecDailyUi(), &RecommendedDaily::playAll,
		[&](auto rhs) { On_NetplayAll(rhs); });

	connect(DicMusic->getRecDailyUi(), &RecommendedDaily::loadOk, this,
		[&] { ui->stackedWidget->setCurrentIndex(12); });

	connect(DicMusic->getRecDailyUi(), &RecommendedDaily::play, this, [&](RecommendedDaily* rhs, const int index) {
		this->On_Netplay(rhs, index);
		});
	connect(DicMusic->getRecDailyUi(), &RecommendedDaily::Nextplay, this, [&](auto rhs, const int index, const QString id) {
		this->On_NetNextPlay(rhs, index, id);
		});

	//歌单歌曲
	connect(DicMusic->getSongMuen(), &SongMenu::DataLoading, this, [&] {
		ui->stackedWidget->setCurrentIndex(16);
		});
}

void Music::SingerDetailsConnect()
{
	connect(singetdeatils, &SingerDetails::loadok, this, [&]() {
		ui->stackedWidget->setCurrentIndex(13);
		singetdeatils->show();
		});

	connect(singetdeatils, &SingerDetails::playAll, this, [&](SingerDetails* rhs) {
		this->On_NetplayAll(rhs);
		});

}

//专辑UI信号
void Music::SongMenuConnect()
{
	connect(DicMusic->getSongMuen(), &SongMenu::SongMenu_playAll, this, [&](SongMenu *rhs) {
		qDebug() << "调用专辑UI信号";
		On_NetplayAll(rhs);
		});
}

void Music::InstallEventFilter() {
	//设置接受鼠标右键
	//ui->SongMenuList->setContextMenuPolicy(Qt::CustomContextMenu);
	//ui->CollectSongMenuList->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->TopWidget->installEventFilter(this);
	ui->lineEdit_search->installEventFilter(this);
	localMusic->getTable()->installEventFilter(this);
	ui->SongMenuList->installEventFilter(this);
	ui->CollectSongMenuList->installEventFilter(this);
}

void Music::init() {
	//配置文件
	config = new Config{};
	//歌词
	lyr = new lyric{};
	//解码
	Decode = new AudioDeCode{};
	//本地音乐
	localMusic = new Local_and_Download{};
	//发现音乐  （BUG）
	DicMusic = new Dicovermusic(this);
	//皮肤
	Skin = new skin(this);
	//登录
	login = new Login(this);
	//个人中心
	perform = new PersonForm(this);
	//搜索
	search = new Search(this);
	search->hide();
	//云盘
	cloudMusic = new CloudMusic(this);
	cloudMusic->hide();
	//初始化先隐藏两个歌单
	ui->CollectSongMenuList->hide();
	ui->SongMenuList->hide();
	singetdeatils = new SingerDetails(this);
	singetdeatils->hide();

	songmenu = new SongMenu(this);
	//我的音乐
	//默认选中第一行
	ui->listWidget->setCurrentRow(0);
	ui->stackedWidget->insertWidget(0, DicMusic);
	ui->stackedWidget->insertWidget(1, new QWidget());
	ui->stackedWidget->insertWidget(2, new QWidget());
	ui->stackedWidget->insertWidget(3, new QWidget());
	ui->stackedWidget->insertWidget(4, new QWidget());
	ui->stackedWidget->insertWidget(5, new QWidget());
	//第二个listwidget
	ui->stackedWidget->insertWidget(6, localMusic);
	ui->stackedWidget->insertWidget(7, new QWidget());
	//添加云盘widegt
	ui->stackedWidget->insertWidget(8, cloudMusic);
	ui->stackedWidget->insertWidget(9, new QWidget());
	ui->stackedWidget->insertWidget(10, new QWidget());

	ui->stackedWidget->insertWidget(11, DicMusic->getSoloAlbumUi());
	ui->stackedWidget->insertWidget(12, DicMusic->getRecDailyUi());
	ui->stackedWidget->insertWidget(13, singetdeatils);
	ui->stackedWidget->insertWidget(14, search);
	ui->stackedWidget->insertWidget(15, songmenu);
	ui->stackedWidget->insertWidget(16, DicMusic->getSongMuen());
	//创建的歌单列表

	//默认音量
	ui->Sli_volum->setMaximum(100);
	ui->Sli_volum->setValue(50);
	//设置无窗口模式
	setWindowFlags(Qt::FramelessWindowHint);
	ui->playslider->setMinimum(0);
	ui->playslider->setMaximum(100);
	//悬停提示
	HoverTip();
}

void Music::initMenu(QListWidget* listwidget)
{
	menu = new QMenu(listwidget);
	Play = new QAction(QIcon(""), "播放", menu);
	NextPlay = new QAction(QIcon(""), "下一首播放", menu);
	menu->addSeparator();
	Down = new QAction(QIcon(""), "下载全部(L)", menu);
	menu->addSeparator();
	Editmenuinfo = new QAction(QIcon(""), "编辑歌单信息", menu);
	DelSongMenu = new QAction(QIcon(""), "删除歌单(Delete)", menu);

}

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
	//鼠标左键按下
	if (event->button() == Qt::LeftButton) {
		//鼠标任意位置按下后，隐藏热搜列表
		search->topsearchlist->hide();

		mMoving = true; //移动中
		//记录位置
		mLastMousePosition = event->globalPos();
	}
}

//鼠标松开
void Music::mouseReleaseEvent(QMouseEvent*) {
	mMoving = false; //没有移动
}

//快捷键
void Music::Presskey(QKeyEvent* event) {
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
bool Music::eventFilter(QObject* obj, QEvent* event) {

	//QListWidget的鼠标右键菜单
	if (obj == ui->SongMenuList) {
		if (event->type() == QEvent::ContextMenu) {
			if (ui->SongMenuList->itemAt(mapFromGlobal(QCursor::pos())) != nullptr) {
				initMenu(ui->SongMenuList);
				//菜单栏在当前鼠标位置弹出
				menu->exec(QCursor::pos());

			}
		}
	}


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
				//更好的做法是发送一个信号，可以获得更多兼容性
				Next(localMusic->PlayerList());
			}
		}
	}
	// ctrl + ← 播放上一首歌曲
	if (obj == localMusic->getTable()) {
		//键盘按下事件
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* evn = static_cast<QKeyEvent*>(event);
			if (evn->key() == Qt::Key_Left) {
				Previous(localMusic->PlayerList());
			}
		}
	}

	//搜索框被点击，显示搜索建议
	if (obj == ui->lineEdit_search) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->buttons() & Qt::LeftButton) {
				//显示热搜列表
				search->TopSearch(this);
			}
		}
	}

	return QMainWindow::eventFilter(obj, event);
}

//推出软件确认
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

void Music::on_title_logo_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
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
	int ms = currentMs1 / 1000 / 60 % 60;
	sec = currentMs1 / 1000 % 60;
	// lyr->getLineByPosition(ms, sec);
	//   qDebug() << ms << ":" << sec << "\n";
	QString currentTime =
		QString("%1 : %2").arg(ms, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
	ui->lab_duration->setText(currentTime);
	if (!sliderSeeking) //未滑动
	{
		ui->playslider->setMaximum(destMs);
		ui->playslider->setValue(currentMs);
	}
	emit updateSongLrc(currentMs1);
}

void Music::on_setPlatList(QStringList list) {
	CurrentPlayerListIndex = 0;
	playlist = list;
}

//滑动滑块
void Music::on_Sli_volum_valueChanged(int value) {
	//设置播放音量
	if (Decode->audio) {
		Decode->audio->setVolume(value / qreal(100.0));
	}
	if (ui->Sli_volum->value() == 0) {
		ui->btn_volum->setStyleSheet(
			"#btn_volum{border-image:url(:/images/bottom/btn_mute.png)}");
	}
	else {
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
	}
	else {
		CurrVolume = ui->Sli_volum->value(); //先记录当前音量
		ui->Sli_volum->setValue(0);
		ui->btn_volum->setStyleSheet(
			"#btn_volum{border-image:url(:/images/bottom/btn_mute.png)}");
	}
}

void Music::on_down_listWidgetClicked(QListWidgetItem* item) {
	switch (ui->down_listWidget->currentRow()) {
	case 2:
		//加载我的云盘数据
		cloudMusic->LoadMyCloud();
	default:
		break;
	}
}

void Music::on_btn_SongMenu_clicked() {
	QString text = config->GetValue("/Pwd/loggingstatus");
	ui->SongMenuList->clear();
	if (text.compare("1") == 0) {
		QStringList menu{ songmenu->getSongMenu() };
		foreach(const QString & rhs, menu) {
			QListWidgetItem* item = new QListWidgetItem(rhs);
			ui->SongMenuList->addItem(item);
		}

		if (ui->SongMenuList->isHidden()) {
			ui->SongMenuList->show();
		}
		else
		{
			ui->SongMenuList->hide();

		}
	}
	else
	{
		//未登录
	}
}

//创建歌单
void Music::on_AddSongMenu_clicked() {
	//    ui->label->setStyleSheet("border-image: url(:/images/btn_down_n.png);");
	QDialog* Creat = new QDialog(this);
	Creat->resize(470, 230);
	Creat->setMaximumSize(470, 230);
	Creat->setMinimumSize(470, 230);
	QLabel* lab = new QLabel("创建歌单", Creat);
	lab->resize(70, 20);
	lab->setStyleSheet("background-color:white; color:black;font:13pt");
	lab->move(205, 30);
	edit = new QLineEdit(Creat);
	edit->resize(440, 30);
	edit->move(20, 85);
	QPushButton* btn = new QPushButton("创建", Creat);
	btn->resize(85, 30);
	btn->move(195, 150);
	btn->setStyleSheet("background-color:#fdbdd3; color:black; font:13pt");
	connect(btn, &QPushButton::clicked, this, [&] {
		if (!edit->text().isEmpty()) {
			DicMusic->getSongMuen()->CreatorSongMuen(edit->text());
		}
		});

	Creat->show();

	connect(DicMusic->getSongMuen(), &SongMenu::CreatorSongMenuOk, this, [&] {
		QListWidgetItem* item = new QListWidgetItem(edit->text());
		ui->SongMenuList->addItem(item);
		Creat->close();
		});
}

void Music::on_btn_collectMenu_clicked() {
	if (ui->CollectSongMenuList->isHidden()) {
		ui->CollectSongMenuList->show();
	}
	else {
		ui->CollectSongMenuList->hide();
	}
}

//我的歌单
void Music::on_SongMenuList_itemClicked(QListWidgetItem* item)
{
	int current = ui->SongMenuList->currentRow();
	songmenu->SongMenuAt(current);
	ui->stackedWidget->setCurrentIndex(15);

}

void Music::on_lineEdit_search_textChanged(QString str) {
	qDebug() << "文字改变\n";
}

//检测当前播放状态
void Music::on_btn_stop_clicked() { CheckState(); }

//上一首
void Music::Previous(QStringList& playerlist) {
	if (!playerlist.isEmpty()) {
		--CurrentPlayerListIndex;
		if (CurrentPlayerListIndex == -1) {
			CurrentPlayerListIndex = playerlist.length() - 1;
		}
		Decode->play(playerlist.at(CurrentPlayerListIndex));
		SetBottonInformation(Decode->tag);
	}
	else {
		QMessageBox::information(this, tr("Error"), tr("播放列表为空!!!!"),
			QMessageBox::Yes);
	}
}
//下一首
void Music::Next(QStringList& playerlist) {
	if (!playerlist.isEmpty()) {
		++CurrentPlayerListIndex;
		if (CurrentPlayerListIndex == playerlist.length()) {
			CurrentPlayerListIndex = 0;
		}
		qDebug() << "CurrentPlayerListIndex" << CurrentPlayerListIndex << "\n";
		Decode->play(playerlist.at(CurrentPlayerListIndex));
		SetBottonInformation(Decode->tag);
	}
	else {
		QMessageBox::information(this, tr("Error"), tr("播放列表为空!!!!"),
			QMessageBox::Yes);
	}
}

void Music::on_btn_prev_clicked() { Previous(playlist); }

void Music::on_btn_next_clicked() { Next(playlist); }

//打开桌面歌词
void Music::on_btn_lyric_clicked() {
	if (lyr->destlyric->isHidden()) {
		lyr->destlyric->show();
	}
	else {
		lyr->destlyric->close();
	}
}

//最大化与还原
void Music::on_btn_max_clicked() {
	static bool max = false;
	static QRect location = this->geometry();
	if (max) {
		this->setGeometry(location); //回复窗口原大小和位置
		ui->btn_max->setIcon(QIcon(":/MAX_.png"));
		max = false;
	}
	else {
		location = this->geometry(); //最大化前记录窗口大小和位置
		ui->btn_max->setIcon(QIcon(":/minMAX.png"));
		// this->showFullScreen(); //设置窗口铺满全屏
		this->showMaximized();
		max = true;
	}
}

//登陆
void Music::on_btn_login_clicked() {
	QString loggingstatus = config->GetValue("Pwd/loggingstatus");
	if (0 == loggingstatus.compare("1")) {
		QPoint skin_xy = QCursor::pos();
		perform->setGeometry(skin_xy.x() - 70, skin_xy.y() + 30, 262, 390);
		perform->show();
	}
	else {
		login->show();
	}
}

//进度条按压处理
void Music::on_playslider_sliderPressed() { sliderSeeking = true; }

//进度条释放处理
void Music::on_playslider_sliderReleased() {
	int value = ui->playslider->value();
	Decode->seek(value);
}

void Music::on_btn_mode_clicked() { setPlayMode(Mode); }

void Music::setPlayMode(PlayMode& Mode) {
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
		Next(playlist);
		return;
		//单曲循环
	case PlayMode::Single:
		break;
	case PlayMode::Random:
		//设置随机种子，即随机数在种子值到32767之间
		qsrand(QTime::currentTime().msec());
		//随机生成0到播放列表的长度的随机数
		CurrentPlayerListIndex = qrand() % playlist.length();
		break;
	}

	Decode->play(playlist.at(CurrentPlayerListIndex));

	// SetBottonInformation(GetTag(tag));
}

void Music::CheckState() {
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
		if (playlist.isEmpty()) {
			qDebug() << "PlayList isempty...\n\n";
			return;
		}
		CurrentPlayerListIndex = 0;
		Decode->play(playlist.at(CurrentPlayerListIndex));
		// SetBottonInformation(tag);
		ui->btn_stop->setStyleSheet(
			"border-image:url(:/images/bottom/btn_pause_h.png);");
		break;
	}
}

void Music::on_btn_pictrue_clicked() {
	// 歌词界面未打开
	if (lyr->isHidden()) {
		ui->scrollArea->hide();
		ui->stackedWidget->hide();
		ui->horizontalLayout_2->addWidget(lyr);
		//search->close();
		lyr->show();
	}
	else {
		lyr->close();
		ui->scrollArea->show();
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
	}
	else {
		Skin->hide();
	}
}

void Music::on_btn_personmessage_clicked() {
	if (config->GetValue("/Pwd/loggingstatus") == "1") {
		if (perform->isHidden()) {
			QPoint skin_xy = QCursor::pos();
			perform->setGeometry(skin_xy.x() - 80, skin_xy.y() + 30, 262, 390);
			perform->show();
		}
		else {
			perform->hide();
		}
	}
	else {
		if (login->isHidden()) {
			login->show();
		}
	}
}

void Music::on_lineEdit_search_returnPressed() {
	//鼠标任意位置按下后，隐藏热搜列表
	search->topsearchlist->hide();
	if (ui->lineEdit_search->text().isEmpty())
		return;
	search->GetSearchText(ui->lineEdit_search->text());
	ui->stackedWidget->setCurrentIndex(14);
}

//播放全部网络歌曲
// void Music::on_NetplayAll() {
//  playlist.clear();
//  CurrentPlayerListIndex = 0;
//  playlist = search->GetPlayList();
//  //需要解析的音乐ID
//  Decode->tag->SetSongId(search->GetPlaylistID());
//  ui->btn_stop->setStyleSheet(
//      "border-image:url(:/images/bottom/btn_pause_h.png);");
//  Decode->play(playlist.at(CurrentPlayerListIndex), 0);
//  qDebug() << "进入次数\n";
//}
//
////播放当前点击的网络歌曲
// void Music::on_Netplay(const int index) {
//  QString url =
//  QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
//                    .arg(search->GetPlaylistID().at(index));
//  Decode->tag->SetSongId(search->GetPlaylistID());
//  playlist.insert(CurrentPlayerListIndex, url);
//  Decode->play(url, index);
//
//  ui->btn_stop->setStyleSheet(
//      "border-image:url(:/images/bottom/btn_pause_h.png);");
//}
//
////当前播放完毕后在播放当前点击的歌曲
// void Music::on_NetNextPlay(const int index, const QString url) {
//  //插入当前播放位置下一个位置，等待播放
//  playlist.insert(CurrentPlayerListIndex + 1, url);
//  Decode->tag->insertUrl(CurrentPlayerListIndex + 1,
//                         search->GetPlaylistID().at(index));
//}

void Music::SetBottonInformation(M_Tag* rhs) {
	ui->btn_pictrue->setIcon(rhs->GetAblueArt());
	ui->btn_pictrue->setIconSize(ui->btn_pictrue->size());
	ui->lab_message->setText(
		QString("%1\n%2").arg(rhs->GetArtist()).arg(rhs->GetTitle()));
	ui->lab_time->setText(rhs->GetDuration());
	lyr->setMessage(rhs->GetAblueArt().toImage(), rhs->GetArtist(),
		rhs->GetTitle());
	SongName = rhs->GetTitle();
	//获取歌词
	lyr->GetTheLyricsName(SongName);
}
