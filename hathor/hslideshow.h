#ifndef HSLIDESHOW_H
#define HSLIDESHOW_H

#include <QWidget>
#include <QList>
#include <QPixmap>
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

    SlideshowItem( QPixmap& pmap ) : QGraphicsPixmapItem(pmap) {}
    virtual ~SlideshowItem() {
        qDebug()<<"DEL SI";
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
    bool s_done;
    bool s_sending;
    QList<int**> s_pri;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
    explicit HSlideshow(HArtist& artist,QWidget* parent=0);

    void hideEvent(QHideEvent * e) {
        pause();
        s_cache.clear();
        while(s_pri.size()) {
            **s_pri.takeFirst()=0;
        }
        QGraphicsView::hideEvent(e);
    }
    void showEvent(QShowEvent *event) {
        s_i=0;
        if(!s_cache.size()) {
            s_pri.push_back(s_artist.sendExtraPics(this,"addPic",15));
            **s_pri.back()=1;
        }
        resume();
        QGraphicsView::showEvent(event);
    }

    QList<QPixmap*> s_cache;
public:
    static HSlideshow* getSlideshow(HArtist&);
public slots:
    void nextPic();
    void pause() { s_done=1; }
    void resume();
    void addPic(QPixmap &p);
};

#endif // HSLIDESHOW_H
