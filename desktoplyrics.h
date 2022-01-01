#ifndef DESKTOPLYRICS_H
#define DESKTOPLYRICS_H

#include <QWidget>

namespace Ui {
class DesktopLyrics;
}

class DesktopLyrics : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopLyrics(QWidget *parent = nullptr);
    ~DesktopLyrics();
    void SetLastlyric(QString str);
    void SetNextlyric(QString str);
private slots:
    void on_btn_close_clicked();

private:
    Ui::DesktopLyrics *ui;
};

#endif // DESKTOPLYRICS_H
