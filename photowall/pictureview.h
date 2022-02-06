#ifndef PICTUREVIEW_H
#define PICTUREVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QAbstractButton>
#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QGraphicsObject>

class pictureView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit pictureView(QWidget *parent = nullptr);
    virtual ~pictureView();
protected:
    virtual void resizeEvent(QResizeEvent *event); //

signals:
    void sizeChanged(const QSize &);
};

#endif // PICTUREVIEW_H
