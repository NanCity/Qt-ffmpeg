
#ifndef BASE_H
#define BASE_H

#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

#include <QAction>
#include <QMenu>
#include <QTableWidget>
#include <QNetworkReply>
class QMenu;

class Base : public QTableWidget {
  Q_OBJECT
public:
  unsigned int column{5};
  explicit Base(QTableWidget *paren = nullptr);
  ~Base();
  QTableWidget *tab;
  void InitTableWidget();
  void DelTableWidgetRow();
  void SerachData(QString search_data);
  void InsertDataInfoTableWidget(const QStringList value, const int index);
  // public slots :
  //	void RightClickMouse(const QPoint& pos);
private:
};
#endif // BASE_H
