#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include <QWidget>
class QLabel;
namespace Ui {
class searchItem;
}

class searchItem : public QWidget
{
    Q_OBJECT

public:
    explicit searchItem(QWidget *parent = nullptr);
    ~searchItem();
    QLabel* getHot();
    QString getName();
    void setNumber(const int num);
    void setName(const QString name);
    void setHot(QPixmap pix);
    void setScore(const int socre);
    void setContent(QString mes);
    void setLabNumColor(bool bol);
  private:
    Ui::searchItem *ui;
};

#endif // SEARCHITEM_H
