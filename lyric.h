#ifndef LYRIC_H
#define LYRIC_H

#include <QWidget>
#include "base.h"
namespace Ui {
class lyric;
}

class lyric : public QWidget
{
    Q_OBJECT
public:
    explicit lyric(QWidget *parent = nullptr);
    ~lyric();

private:
    Ui::lyric *ui;
};

#endif // LYRIC_H
