#ifndef HSLIDESHOW_H
#define HSLIDESHOW_H

#include <QWidget>
#include <QList>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTime>
#include <QPropertyAnimation>
#include "htrack.h"
#include "hartist.h"

class SlideshowItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal echoOpacity READ echoOpacity WRITE setEchoOpacity)
    Q_PROPERTY(qreal echoScale READ echoScale WRITE setEchoScale)
    Q_PROPERTY(qreal echoX READ echoX WRITE setEchoX)
    Q_PROPERTY(qreal echoY READ echoY WRITE setEchoY)
    QTime wt,xt,yt,zt;
    int* s_pri;
public slots:
    void setEchoOpacity(qreal opacity){
        if(wt.msecsTo(QTime::currentTime())<40) return;
        wt=QTime::currentTime();
        QGraphicsPixmapItem::setOpacity(opacity);
    }
    void setEchoScale(qreal scale){
        if(xt.msecsTo(QTime::currentTime())<30) return;
        xt=QTime::currentTime();
        QGraphicsPixmapItem::setScale(scale);
    }
    void setEchoX(qreal x) {
        if(yt.msecsTo(QTime::currentTime())<30) return;
        yt=QTime::currentTime();
        setPos(x,y());
    }
    void setEchoY(qreal y) {
        if(zt.msecsTo(QTime::currentTime())<30) return;
        zt=QTime::currentTime();
        setPos(x(),y);
    }

public:
    qreal echoOpacity() { return opacity(); }
    qreal echoScale() { return scale(); }
    qreal echoX() { return x(); }
    qreal echoY() { return y(); }

    SlideshowItem( QImage& pmap,int* pri ) : QGraphicsPixmapItem(QPixmap::fromImage(pmap)),s_pri(pri) {}
    virtual ~SlideshowItem() {
        setPixmap(0);
        scene()->removeItem(this);
        *s_pri=0;
        HCachedPixmap::release();
    }
};

class HSlideshow : public QGraphicsView
{
    Q_OBJECT
    static QHash<QString, HSlideshow*> s_u;
    HArtist& s_artist;
    QGraphicsScene sc;
    int s_i;
    int s_z;
    bool s_sending;
    QTime s_t;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
    explicit HSlideshow(HArtist& artist,QWidget* parent=0);

    void hideEvent(QHideEvent * e) {
        s_cur=0;
        s_curPri=0;
        QGraphicsView::hideEvent(e);
    }
    void showEvent(QShowEvent *event) {
        resume();
        QGraphicsView::showEvent(event);
    }

    int* s_curPri;
    QImage* s_cur;
public:
    static HSlideshow* getSlideshow(HArtist&);
    virtual ~HSlideshow() {
        s_u.remove(s_artist.getName());
    }
public slots:
    void nextPic();
    void resume();
    void addPic(QImage &p);
    void addPic(QImage *p);
};

#endif // HSLIDESHOW_H
