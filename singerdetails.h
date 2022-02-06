#ifndef SINGERDETAILS_H
#define SINGERDETAILS_H

#include <QWidget>
class Base;
class Singer;
class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
class SingerDetails;
}

class SingerDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SingerDetails(QWidget *parent = nullptr);
    ~SingerDetails();
    void setmesg(Singer singer);
    void setID(const int id);
   QStringList GetPlayList() { return songid; }
    //QList<unsigned int> GetPlaylistID() { return ID; }
protected slots:
    void on_btn_playAll_clicked();
    void finsedNetSingerDet(QNetworkReply* reply);
    void finshedNetTop50(QNetworkReply* reply);
signals:
    void playAll(SingerDetails* );
    void play(SingerDetails*, const int index);
    void Nextplay(SingerDetails*, const int index, const QString ID);
    void loadok();
private:
    Ui::SingerDetails *ui;
    int singerID{};
    Base* base;
    //QList<unsigned int> ID{};
    QStringList songid{ };
    QNetworkAccessManager* NetSingerDet;
    QNetworkAccessManager* NetTop50;
};

#endif // SINGERDETAILS_H
