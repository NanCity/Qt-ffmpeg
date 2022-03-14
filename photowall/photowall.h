#ifndef PHOTOWALL_H
#define PHOTOWALL_H

#include <QWidget>
#include <QQueue>
#define RAW_VIEW_SIZE QSize(750,198)
#define SCALE_VIEW_PIXMAP (qreal)1/1 //View与图片比例
#define SCALE_BIG_SMALL (qreal)1/1 //图片大小比例
//P1-P10，10个位置，根据需要改动
#define P1 (qreal)0.00
#define P2 (qreal)0.15
#define P3 (qreal)0.44
#define P4 (qreal)0.15
#define P5 (qreal)0.15
#define P6 (qreal)0.15
#define P7 P2
#define P8 P2
#define P9 P2
#define P10 P2
#define MID_TYPE 1
#define FPS 60//帧数，每秒
#define DURATION_MS 500//移动一次图元经过时间,毫秒，不得低于帧数

class QButtonGroup;
class QGraphicsScene;
class QNetworkReply;
class QGraphicsItemAnimation;
class QNetworkAccessManager;

namespace Ui {
	class PhotoWall;
}

//轮播图区分专辑和单曲
struct Target {
	int targetId;
	QString typeTitle;
	QString picUrl;
	Target() : targetId{}, typeTitle{}, picUrl{} {}
};

class PhotoWall : public QWidget
{
	Q_OBJECT
public:
	enum Rules :int {
		RuleA = 1,
		RuleB = -1,
		RuleC = 2,
		RuleD = -2
	};
public:
	explicit PhotoWall(QWidget* parent = nullptr);
	virtual ~PhotoWall();
	//设置button组
	void setButtonGroup();
	//设置相关图片信息
	void setInitList();
	//加载图片
	void setPictureScreen();
	//设置定时器
	void setTimerAndConnect();
	template<typename T>
	void rollList(QList<T>& oldList, int dir, int count);
	void rollItem(int rollDir, unsigned rollCount);
	//获取准确的位置
	int  getrightN(int num);

protected slots:
	void timerOutFunc();
	//点击滚动
	void clickedItemRoll(int type);
	void on_btnL_clicked();
	void on_btnR_clicked();

	void on_finshedNetMangBanner(QNetworkReply* reply);
	void on_finshedNetGetBanner(QNetworkReply* reply);
private:
	Ui::PhotoWall* ui;
	//定时器
	QTimer* m_timer;
	//场景
	QGraphicsScene* m_scene;
	//    PictrueView *m_view; //视图
	QLineF m_MidLine{}; //中等线，确定图片位置
	//各个图片位置信息
	QList<qreal> m_PointList{};
	//各个图片列表
	QList<QPixmap> m_PixmapList{};
	//各个显示优先级列表
	QList<qreal> m_ZValueList{};
	//各个图片位置信息伸缩因子列表
	QList<qreal> m_PixmapScaleList{};
	int m_index{};
	//当前执行的类型操作
	Rules m_currentRule;
	//滚动次数
	unsigned m_rollCount{};
	//按钮盒子
	QButtonGroup* m_BtnGroup{};
	bool btnMoveEnable{};
	QVector<QGraphicsItemAnimation*>item{ 10 };
	QTimer* m_newT{}; //旋转定时器

	//获取网易云音乐的照片墙图片
	int index{ 0 };
	QList<Target> targetlist;
	QNetworkAccessManager* NetMangBanner;
	//解析图片
	QNetworkAccessManager* NetGetBanner;
};



#endif // PHOTOWALL_H
