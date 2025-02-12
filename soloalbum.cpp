﻿#include "soloalbum.h"
#include "base.h"
#include "config.h"
#include "ui_soloalbum.h"
#include <QHBoxLayout> //水平布局管理器
SoloAlbum::SoloAlbum(QWidget* parent) : QWidget(parent), ui(new Ui::SoloAlbum) {
	ui->setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose);
	base = new Base(ui->playlist);
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

void SoloAlbum::Init() {
	ui->playlist->resize(ui->tabWidget->size());
	ui->tabWidget->tabBar()->setTabText(0, "歌曲列表");
	ui->tabWidget->tabBar()->setTabText(1, "专辑详情");
	QStringList head{ "操作", "音乐标题", "歌手", "专辑", "时长" };
	int len = head.length();
	ui->playlist->setColumnCount(len);
	for (int i = 0; i != len; i++) {
		ui->playlist->setHorizontalHeaderItem(i, new QTableWidgetItem(head.at(i)));
	}
	//设置第一列表头自适应widget宽高
	ui->playlist->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

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
		emit Alb_Nextplay(this, index, songID.at(index));
		});
	//下载
	connect(Actdownload, &QAction::triggered, this, [&]() {});
}

void SoloAlbum::setAlbumtag(Albumtag& t) { Albtag.push_back(t); }

void SoloAlbum::LoadData() {
	const int len = Albtag.length();
	songID.clear();
	for (int x = 0; x != len; x++) {
		//插入新的一行
		ui->playlist->insertRow(x);
		if (base->isLike(Albtag.at(x).Song_id)) {
			ui->playlist->setCellWidget(x, 0, base->setItemWidget(1));
		}
		else
		{
			ui->playlist->setCellWidget(x, 0, base->setItemWidget(0));
		}
		QTableWidgetItem* item1 = new QTableWidgetItem(Albtag.at(x).Title);
		QTableWidgetItem* item2 = new QTableWidgetItem(Albtag.at(x).Artist);
		QTableWidgetItem* item3 = new QTableWidgetItem(Albtag.at(x).Album);
		QTableWidgetItem* item4 = new QTableWidgetItem(Albtag.at(x).Duration);
		ui->playlist->setItem(x, 1, item1);
		ui->playlist->setItem(x, 2, item2);
		ui->playlist->setItem(x, 3, item3);
		ui->playlist->setItem(x, 4, item4);
		songID.push_back(QString::number(Albtag.at(x).Song_id));
	}

	emit loadOk();
}
//鼠标右键
void SoloAlbum::on_playlist_customContextMenuRequested(const QPoint& pos) {
	if (ui->playlist->itemAt(pos) != nullptr) {
		menu->exec(QCursor::pos());
	}
}