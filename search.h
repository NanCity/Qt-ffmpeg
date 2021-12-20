#ifndef SEARCH_H
#define SEARCH_H

#include <QWidget>

namespace Ui {
class Search;
}

class Search : public QWidget
{
    Q_OBJECT

public:
    explicit Search(QWidget *parent = nullptr);
    ~Search();
    void InitPlayListTabWiget();
  private slots:
    void on_btn_playall_clicked();

private:
    Ui::Search *ui;
};

#endif // SEARCH_H
