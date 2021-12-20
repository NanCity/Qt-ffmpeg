#ifndef PERSONFORM_H
#define PERSONFORM_H

#include <QWidget>

namespace Ui {
class PersonForm;
}

class PersonForm : public QWidget
{
    Q_OBJECT

public:
    explicit PersonForm(QWidget *parent = nullptr);
    ~PersonForm();
    void initListWidget();
private slots:
    void leaveEvent(QEvent* event);
private:
    Ui::PersonForm *ui;
};

#endif // PERSONFORM_H
