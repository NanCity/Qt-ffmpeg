
#ifndef BASE_H
#define BASE_H

#include <QAction>
#include <QMenu>
#include <QTableWidget>
class QMenu;

class Base :public QTableWidget {
	Q_OBJECT
public:
	explicit Base(QTableWidget* paren = nullptr);
	~Base();
	void InitTableWidget();
	void DelTableWidgetRow();
	void SerachData(QString search_data);
	QTableWidget* tab;
//public slots :
//	void RightClickMouse(const QPoint& pos);
//private:

};

#endif // BASE_H
