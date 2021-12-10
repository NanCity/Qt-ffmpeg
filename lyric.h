#ifndef LYRIC_H
#define LYRIC_H

#include <QWidget>
#include <QMouseEvent>
namespace Ui {
class lyric;
}

class QLabel;
class QMenu;

class lyric : public QWidget
{
    Q_OBJECT
public:
    explicit lyric(QWidget *parent = nullptr);
    ~lyric();
    void setMessage(QImage &img, QString Art, QString& title);
private:
    Ui::lyric *ui;
};

#endif // LYRIC_H
