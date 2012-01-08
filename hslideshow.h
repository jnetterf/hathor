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
    Q_PROPERTY(qreal echoX READ echoX WRITE setEchoX)
    Q_PROPERTY(qreal echoY READ echoY WRITE setEchoY)
public slots:
    void setEchoOpacity(qreal opacity){QGraphicsPixmapItem::setOpacity(opacity);}
    void setEchoScale(qreal scale){
        QGraphicsPixmapItem::setScale(scale);
    }
    void setEchoX(qreal x) {
        setPos(x,y());
    }
    void setEchoY(qreal y) {
        setPos(x(),y);
    }

public:
    qreal echoOpacity() { return opacity(); }
    qreal echoScale() { return scale(); }
    qreal echoX() { return x(); }
    qreal echoY() { return y(); }

    SlideshowItem( QPixmap pmap ) : QGraphicsPixmapItem(pmap) {}
};

class HSlideshow : public QGraphicsView
{
    Q_OBJECT
    HTrack& s_track;
    QGraphicsScene sc;
    int s_i;
    int s_z;
    bool s_done;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
public:
    explicit HSlideshow(HTrack& track,QWidget* parent=0);
public slots:
    void newPic();
    void deleteWhenPossible() { s_done=1; }
};

#endif // HSLIDESHOW_H
