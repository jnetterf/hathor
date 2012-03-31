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


class HSlideshow : public QWidget
{
    Q_OBJECT
    static QHash<QString, HSlideshow*> s_u;
    HArtist& s_artist;
    int s_i;
    int s_z;
    bool s_sending;
    QTime s_t;
    QSize minimumSizeHint() const {
        return sizeHint();
    }
    explicit HSlideshow(HArtist& artist,QWidget* parent=0);

    void hideEvent(QHideEvent * e) {
        while(s_cur.size()) { s_cur[0]=0; s_cur.removeFirst(); }
        while(s_curPri.size()) { s_curPri[0]=0; s_curPri.removeFirst(); }
        QWidget::hideEvent(e);
    }
    void showEvent(QShowEvent *event) {
        resume();
        QWidget::showEvent(event);
    }

    void paintEvent(QPaintEvent *);

    QList<int*> s_curPri;
    QList<QImage*> s_cur;
    QList<QTime> s_curTime;
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
