﻿#ifndef SKIN_H
#define SKIN_H
#include <QByteArray>
#include <QDialog>
#include <QFile>
class QDebug;

#define ERROR(rhs)                                                             \
  qDebug() << "FileName: " << __FILE__ << " line: " << __LINE__ << ' ' << rhs  \
           << endl;

namespace Ui {
class skin;
}

class skin : public QDialog {
  Q_OBJECT

public:
  explicit skin(QWidget *parent = nullptr);
  virtual ~skin();
  void init();
  //void OpenQssFile(QString path);
signals:
  void setThem(QByteArray aray);

protected:
  void leaveEvent(QEvent *event);
private slots:
  void on_btn_black_clicked();

  void on_btn_red_clicked();

  void on_btn_pin_clicked();

  void on_btn_blue_clicked();

  void on_btn_green_clicked();

  void on_btn_gold_clicked();


private:
  Ui::skin *ui;
};

#endif // SKIN_H
