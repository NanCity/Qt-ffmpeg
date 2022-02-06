#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "recommendeddaily.h"
#include "base.h"
#include "tag.h"
#include "ui_recommendeddaily.h"
RecommendedDaily::RecommendedDaily(QWidget* parent)
	: QWidget(parent), ui(new Ui::RecommendedDaily) {
	ui->setupUi(this);
	base = new Base(ui->table_songslist);
	init();


	//菜单栏按钮
	connect(base->Actplay, &QAction::triggered, this,
		[&]() {
			int current = ui->table_songslist->currentRow();
			emit play(this, current);
		});

	connect(base->Actnextplay, &QAction::triggered, this, [&]() {
		int current = ui->table_songslist->currentRow();
		emit Nextplay(this, current, PlayerList.at(current));
		});

	//收藏
	connect(base->Actcollect, &QAction::triggered, this, [&]() {});
	//下载
	connect(base->Actdownload, &QAction::triggered, this, [&]() {});
}

RecommendedDaily::~RecommendedDaily() { delete ui; }

void RecommendedDaily::init() {
	QStringList title{ "操作", "音乐标题", "歌手", "专辑", "时长" };
	int count = title.length();
	ui->table_songslist->setColumnCount(count);
	for (int i = 0; i != count; ++i) {
		ui->table_songslist->setHorizontalHeaderItem(
			i, new QTableWidgetItem(title.at(i)));
	}

	//设置第一列表头不可拉伸
	ui->table_songslist->horizontalHeader()->setSectionResizeMode(
		0, QHeaderView::Fixed);
}

void RecommendedDaily::loadData(QList<Temptag>& rhs) {
	int index = 0;
	//songsid.clear();
	PlayerList.clear();
	foreach(const Temptag & rhs, rhs) {
		ui->table_songslist->insertRow(index);
		ui->table_songslist->setCellWidget(index, 0, base->setItemWidget());
		QTableWidgetItem* item1 = new QTableWidgetItem(rhs.Title);
		QTableWidgetItem* item2 = new QTableWidgetItem(rhs.Artist);
		QTableWidgetItem* item3 = new QTableWidgetItem(rhs.Ablue);
		QTableWidgetItem* item4 = new QTableWidgetItem(rhs.Duration);
		ui->table_songslist->setItem(index, 1, item1);
		ui->table_songslist->setItem(index, 2, item2);
		ui->table_songslist->setItem(index, 3, item3);
		ui->table_songslist->setItem(index, 4, item4);
		//songsid.push_back();
	   /* QString Url{QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
						.arg(rhs.Songid)};*/
		PlayerList.insert(index, QString::number(rhs.Songid));
		++index;
	}
	//数据加载完成
	emit loadOk();
}
//账号未登录
void RecommendedDaily::Notlogin(bool login) {
	QLabel* lab = new QLabel("未登录账号", ui->table_songslist);
	lab->move(ui->table_songslist->width() / 2,
		(ui->table_songslist->height() / 2));
	lab->show();
}
void RecommendedDaily::on_btn_playAll_clicked() { emit playAll(this); }

void RecommendedDaily::on_btn_collectAll_clicked() {}