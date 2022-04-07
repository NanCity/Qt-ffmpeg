#include "photowall.h"
#include "ui_photowall.h"
#include "picturebutton.h"
#include "pictureview.h"
#include <QPixmap>
#include <QPainterPath>
#include <QGraphicsView>
#include <QDebug>
#include <QTimeLine>
#include <QTransform>
#include <QButtonGroup>
#include <QMap>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsItemAnimation>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>

static int dir = 0;//记录方向
static QList<qreal> spaceList;
static QList<qreal> unitList;
static QList<qreal> transScaleList;//缩放比例表
static QSize pictrueBigSize = RAW_VIEW_SIZE / SCALE_VIEW_PIXMAP;
static QSize pictrueSmallSize = RAW_VIEW_SIZE / SCALE_VIEW_PIXMAP / SCALE_BIG_SMALL;
static QList<pictureItem*> itemList;
static QList<int> finishList;
static QMap<int, pictureItem*> mapLink;  //按钮id 与 图片资源的映射
static int startNum = 0;
static QList<QPointF> pointA;

PhotoWall::PhotoWall(QWidget* parent) : QWidget(parent)
, ui(new Ui::PhotoWall)
{
	ui->setupUi(this);
	this->installEventFilter(this);
	for (int i = 0; i < 10; i++) {
		item[i] = new QGraphicsItemAnimation(this);
	}
	m_timer = new QTimer(this);
	m_scene = new QGraphicsScene(this);
	NetMangBanner = new QNetworkAccessManager(this);
	/*
	* type = 0 表示PC端
	*/
	QString url{ "http://localhost:3000/banner?type=0" };
	NetMangBanner->get(QNetworkRequest(url));
	connect(NetMangBanner, &QNetworkAccessManager::finished, this, &PhotoWall::on_finshedNetMangBanner);

	NetGetBanner = new QNetworkAccessManager(this);
	connect(NetGetBanner, &QNetworkAccessManager::finished, this, &PhotoWall::on_finshedNetGetBanner);


	m_index = 0;
	m_currentRule = RuleA;
	m_rollCount = 0;
	btnMoveEnable = true;
	setAttribute(Qt::WA_StyledBackground); //设置样式表
	setButtonGroup(); //设置按钮组
	setInitList();

	m_newT = new QTimer(this);
	connect(m_newT, &QTimer::timeout, [this]() {
		on_btnR_clicked();
		});
	m_newT->setInterval(5000);
	m_newT->start();

	ui->btnL->hide();
	ui->btnR->hide();
}

PhotoWall::~PhotoWall()
{
	m_newT->stop();
	delete ui;
}


void PhotoWall::on_finshedNetMangBanner(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray byt = reply->readAll();
		QJsonParseError error_t{};
		QJsonDocument docm = QJsonDocument::fromJson(byt, &error_t);
		if (error_t.error == QJsonParseError::NoError) {
			targetlist.clear();
			QJsonObject root = docm.object();
			auto Banrot = root.value("banners");
			if (Banrot.isArray()) {
				QJsonArray BanArry = Banrot.toArray();
				foreach(const auto & x, BanArry) {
					if (x.isObject()) {
						Target target{};
						auto Ban_r = x.toObject();
						target.targetId = Ban_r.value("targetId").toInt();
						target.typeTitle = Ban_r.value("typeTitle").toString();
						target.picUrl = Ban_r.value("imageUrl").toString();
						//取得轮播图的链接
						targetlist.push_back(target);
						NetGetBanner->get(QNetworkRequest(target.picUrl));
					}
				}
			}
		}
	}
	reply->deleteLater();
}


void PhotoWall::on_finshedNetGetBanner(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		static int i = 0;
		QPixmap map{};
		++index;
		++i;
		map.loadFromData(reply->readAll());
		map.save(QString("../photowall/pictrue/%1.png").arg(index));
		if (i == targetlist.length()) {
			//加载图片
			setPictureScreen();
			setTimerAndConnect();
		}
	}
	reply->deleteLater();
}

void PhotoWall::setButtonGroup()
{
	m_BtnGroup = new QButtonGroup(this);
	m_BtnGroup->addButton(ui->btnPic0, 0);
	m_BtnGroup->addButton(ui->btnPic1, 1);
	m_BtnGroup->addButton(ui->btnPic2, 2);
	m_BtnGroup->addButton(ui->btnPic3, 3);
	m_BtnGroup->addButton(ui->btnPic4, 4);
	m_BtnGroup->addButton(ui->btnPic5, 5);
	m_BtnGroup->addButton(ui->btnPic6, 6);
	m_BtnGroup->addButton(ui->btnPic7, 7);
	m_BtnGroup->addButton(ui->btnPic8, 8);
	m_BtnGroup->addButton(ui->btnPic9, 9);
	m_BtnGroup->setExclusive(true);
	//起始位置
	m_BtnGroup->button(0)->setChecked(true);
	for (int i = 0; i < 10; i++) {
		static_cast<pictureButton*>(m_BtnGroup->button(i))->setId(i);
	}
	int n = 0;
}

void PhotoWall::setInitList()
{
	m_PointList << P1 << P2 << P3 << P4 << P5 << P6 << P7 << P8 << P9 << P10;
	m_ZValueList << 1 << 2 << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
	m_PixmapScaleList << 0.8 << 1 << 0.8 << 0.8 << 0.8 << 0.8 << 0.8 << 0.8 << 0.8 << 0.8;
}

void PhotoWall::setPictureScreen()
{
	//设置样式 无标边框
	ui->graphicsView->setStyleSheet("background: transparent; padding: 0px; border: 0px;");
	ui->graphicsView->setScene(m_scene);
	m_scene->setSceneRect(0, 0, RAW_VIEW_SIZE.width(), RAW_VIEW_SIZE.height());
	m_MidLine.setPoints(QPointF(0, 0), \
		QPointF(RAW_VIEW_SIZE.width(), 0));
	//添加对应图片
	for (int i = 1; i <= index; i++) {
		m_PixmapList.append(QPixmap(QString("../photowall/pictrue/%1.png").arg(i)));
	}
	for (int i = 0; i < index; i++) {
		itemList.append(new pictureItem(m_PixmapList[i].scaled(pictrueBigSize,
			Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		itemList[i]->setScale(m_PixmapScaleList[i]);
		itemList[i]->setType(i);
		itemList[i]->setItemId(i);
		itemList[i]->setOffset(QPointF(0, 0));
	}
	//图元添加图片
	for (int i = 0; i < index; i++)
	{
		m_scene->addItem(itemList[i]);//添加图元
		itemList[i]->setPos(m_MidLine.pointAt(m_PointList[i]));//设置位置
		itemList[i]->setZValue(m_ZValueList[i]);//设置显示优先级
		itemList[i]->setTransformationMode(Qt::SmoothTransformation);//设置缩放模式
		if (i != 1)
		{
			itemList[i]->setPos(itemList[i]->x(), RAW_VIEW_SIZE.height() / 10);
		}
		pointA.append(itemList[i]->pos());  //放入到对应位置
		qDebug() << pointA[i].x() << ": " << pointA[i].y();
	}
	//放入映射map中
	for (int i = 0; i < index; i++)
	{
		mapLink.insert(static_cast<pictureButton*>(m_BtnGroup->button(i))->id(), itemList[i]);
	}

	//打印信息
	//    QMap<int,pictureItem *>::const_iterator it;

	//    for (it = mapLink.constBegin();it!=mapLink.constEnd();it++) {
	//        qDebug()<< it.key()<< ": " << it.value();
	//    }

}

void PhotoWall::setTimerAndConnect()
{
	//利用持续时间和帧数计算出定时时间,持续时间/（帧数*持续时间），这里乘1000是转为秒
	m_timer->setInterval(DURATION_MS / (FPS * DURATION_MS / 1000));
	connect(m_timer, &QTimer::timeout, this, &PhotoWall::timerOutFunc);
	for (int i = 0; i < index; i++)
	{
		connect(itemList[i], &pictureItem::clickedId, this, &PhotoWall::clickedItemRoll);

		void (pictureButton:: * funcPtr)(int) = &pictureButton::entered;
		connect(static_cast<pictureButton*>(m_BtnGroup->button(i)), funcPtr, [this](int id) {
			pictureItem* p = mapLink.value(id); //获取id 的图片值
			btnMoveEnable = false;
			clickedItemRoll(p->type());
			//qDebug() << p->type();
			});
		//        connect(static_cast<pictureButton*>(m_BtnGroup->button(i)),&pictureButton::stop,[this](){
		//            m_timer->stop();
		//            //            m_timer->
		//            qDebug()<<"暂停\n";
		//        });
	}

}

void PhotoWall::rollItem(int rollDir, unsigned rollCount)
{
	rollCount = 0;
	if (m_timer->isActive())
		return;
	//获取新的数据
	//主要位置  主位置
	int nbegin = rollDir;
	//qDebug() << "nbegin = " << nbegin << "title= " << targetlist.at(nbegin).typeTitle;
	startNum = getrightN(nbegin);
	m_timer->start();
}

int PhotoWall::getrightN(int num)
{
	if (num == -1)
	{
		//num = 9;
		num = index - 1;
	}
	//if(num == 10)
	if (num == index)
	{
		num = 0;
	}

	return num;
}


void PhotoWall::timerOutFunc()
{
	QTimeLine* timeline = new QTimeLine(200);
	timeline->setLoopCount(1); //设置3次
	int first = getrightN(startNum - 1);

	for (int i = 0; i < index; i++) {
		itemList[i] = mapLink.value(first % index);
		first++;
		itemList[i]->setScale(m_PixmapScaleList[i]);
		itemList[i]->setZValue(m_ZValueList[i]);//设置显示优先级
		itemList[i]->setTransformationMode(Qt::SmoothTransformation);//设置缩放模式
		item[i]->setItem(itemList[i]);
		item[i]->setTimeLine(timeline);
		item[i]->setPosAt(1, pointA[i]);
		itemList[i]->setPos(pointA[i]);
	}
	timeline->start();
	m_scene->invalidate();
	m_timer->stop();
}

void PhotoWall::clickedItemRoll(int type)
{
	//查看定时器是否运行
	if (m_timer->isActive())
		return;
	rollItem(type, 0); //得到最新的id 按钮位置
}

//左边运动
void PhotoWall::on_btnL_clicked()
{
	int id = m_BtnGroup->checkedId();
	if (id - 1 < 0)
	{
		//id = 8;
		id = index - 2;
	}
	else {
		id = id - 1;
	}
	m_BtnGroup->button(id)->setChecked(true);
	rollItem(id, 0);
}
//右边运动
void PhotoWall::on_btnR_clicked()
{
	if (index == 0)return;
	int id = m_BtnGroup->checkedId();
	id = (id + 1) % index;
	m_BtnGroup->button(id)->setChecked(true);
	rollItem(id, 0);
}


template<typename T>
void PhotoWall::rollList(QList<T>& oldList, int dir, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (dir == 1)
		{
			oldList.prepend(oldList.last());
			oldList.removeLast();
		}
		else if (dir == -1)
		{
			oldList.append(oldList.first());
			oldList.removeFirst();
		}
	}

}
