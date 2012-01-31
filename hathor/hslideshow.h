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
    static QHash<QString, HSlideshow*> s_u;
    HTrack& s_track;
    QGraphicsScene sc;
    int s_i;
    int s_z;
    bool s_done;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
    explicit HSlideshow(HTrack& track,QWidget* parent=0);
public:
    static HSlideshow* getSlideshow(HTrack&);
public slots:
    void newPic();
    void pause() { s_done=1; }
    void resume() { if(!s_done) { s_done=0; QTimer::singleShot(0,this,SLOT(newPic())); } }
};

#endif // HSLIDESHOW_H
