#ifndef BASE_H
#define BASE_H
#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include <QAction>
#include <QMenu>
#include <QNetworkReply>
#include <QTableWidget>
#include "config.h"
#include "Loading.h"
class QMenu;
class Loading;
class QWheelEvent;
class QNetworkReply;
class QNetworkAccessManager;

class Base : public QTableWidget {
	Q_OBJECT
protected:
	//事件过滤器
	bool eventFilter(QObject* obj, QEvent* event);
public:
	unsigned int column{ 5 };
	explicit Base(QTableWidget* paren = nullptr);
	virtual ~Base();
	void loadMovie();
	void closeMovie();
	//初始化表格
	void InitTableWidget();
	//删除表格中的内容
	void DelTableWidgetRow();
	//检索表格中的内容
	void SerachData(QString search_data);
	//插入数据到表格
	void InsertDataInfoTableWidget(const QStringList value, const int index);
	//添加tablewidget小部件
	QWidget* setItemWidget(int statue = 0);
	void Download(const unsigned int in);
	//创建一个右键菜单
	void CreatorMenu();
	//自定义菜单按钮
	void CreatorMenu(const QList<QAction*> Act);
	void GetMyLikeMusicID();
	//是否位喜欢的歌曲
	bool isLike(const int ID);
protected slots:
	void RightClickMouse(const QPoint& pos);
	void on_FinshedNetMyLikeMusicId(QNetworkReply*);
signals:
	void loadNextPage();
private:
	//加载动画
	Loading* movie;
	Config config;
	//表格中的小部件
	QWidget* widget{};
	QTableWidget* tab;
	QList<int> likeMusicId{};
	//我喜欢的音乐 -- ID
	QNetworkAccessManager* NetMyLikeMusicId;
public:

	//菜单
	QMenu* menu;
	//评论
	QAction* Actcomment;
	//播放当前点击
	QAction* Actplay;
	//下一首播放
	QAction* Actnextplay;
	//收藏
	QAction* Actcollect;
	//下载
	QAction* Actdownload;
};

#endif // BASE_H
