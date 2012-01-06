#ifndef HSLIDESHOW_H
#define HSLIDESHOW_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include "htrack.h"
#include "hartist.h"

class SlideshowItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal echoOpacity READ echoOpacity WRITE setEchoOpacity)
    Q_PROPERTY(qreal echoScale READ echoScale WRITE setEchoScale)
public slots:
    void setEchoOpacity(qreal opacity){QGraphicsPixmapItem::setOpacity(opacity);}
    void setEchoScale(qreal scale){
        QGraphicsPixmapItem::setScale(scale);
    }
public:
    qreal echoOpacity() { return opacity(); }
    qreal echoScale() { return scale(); }

    SlideshowItem( QPixmap pmap ) : QGraphicsPixmapItem(pmap) {}
};

class HSlideshow : public QGraphicsView
{
    Q_OBJECT
    HTrack& s_track;
    QGraphicsScene sc;
    int s_i;
public:
    explicit HSlideshow(HTrack& track,QWidget* parent=0);
public slots:
    void newPic();
};

#endif // HSLIDESHOW_H
