#ifndef RECOMMENDEDDAILY_H
#define RECOMMENDEDDAILY_H

//每日推荐列表
#include <QWidget>
class Base;
class Temptag;
namespace Ui {
class RecommendedDaily;
}

class RecommendedDaily : public QWidget {
  Q_OBJECT

public:
  explicit RecommendedDaily(QWidget *parent = nullptr);
  ~RecommendedDaily();
  void init();
  void loadData(QList<Temptag> *rhs);
  void Notlogin(bool login = false);
  QStringList GetPlayList() { return PlayerList; }
protected slots:
  void on_btn_playAll_clicked();
  void on_btn_collectAll_clicked();
signals:
  void loadOk();
  void play(RecommendedDaily *, const int index);
  void playAll(RecommendedDaily *);
  void Nextplay(RecommendedDaily *, const int index, const QString ID);

private:
  Ui::RecommendedDaily *ui;
  Base *base;
  QStringList PlayerList{};
};

#endif // RECOMMENDEDDAILY_H
