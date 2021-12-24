#ifndef LYRIC_H
#define LYRIC_H

#include <QMouseEvent>
#include <QWidget>
namespace Ui {
class lyric;
}

class QLabel;
class QMenu;

class lyric : public QWidget {
  Q_OBJECT
public:
  explicit lyric(QWidget *parent = nullptr);
  virtual ~lyric();
  void setMessage(QImage &img, QString Art, QString &title);

private:
  Ui::lyric *ui;
};

#endif // LYRIC_H
