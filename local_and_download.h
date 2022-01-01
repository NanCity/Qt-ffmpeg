
#ifndef LOCAL_AND_DOWNLOAD_H
#define LOCAL_AND_DOWNLOAD_H

#include "Decode.h"
#include "base.h"
#include <QWidget>
namespace Ui {
class Local_and_Download;
}

class Local_and_Download : public QWidget {
  Q_OBJECT

public:
  explicit Local_and_Download(QWidget *parent = nullptr);
  virtual ~Local_and_Download();
  QStringList &PlayerList() { return lists; };
  QTableWidget *getTable();
  void Menu();
  void InitTableHeader();

private slots:
  void on_btn_openFile_clicked();
  void on_tableWidget_customContextMenuRequested(const QPoint &pos);
signals:
  void t_play(const int index);
  void t_nextplay(const int index);
  void t_loaded(QStringList list);
  void t_delete(const int n);
private:
  Base *base{};
  QList<QAction *> listAct{};
  QMenu *menu;
  QAction *play;
  QAction *nextplay;
  QAction *Del;
  //解码
  AudioDeCode code{};
  QStringList lists{};
  Ui::Local_and_Download *ui;
};

#endif // LOCAL_AND_DOWNLOAD_H
