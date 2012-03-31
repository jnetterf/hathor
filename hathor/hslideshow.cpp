#include "hslideshow.h"
#include <QPropertyAnimation>

QHash<QString, HSlideshow*> HSlideshow::s_u;

HSlideshow* HSlideshow::getSlideshow(HArtist &t) {
    QString dumbName=t.getName();
    if(s_u.contains(dumbName)) {
        return s_u[dumbName];
    } else {
        return s_u[dumbName]=new HSlideshow(t);
    }
}

HSlideshow::HSlideshow(HArtist &artist, QWidget *parent) : QWidget(parent), s_artist(artist), s_i(0), s_z(0), s_sending(0), s_t()
{
    setFixedWidth(900);
    setFixedHeight(600);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setBackgroundRole(QPalette::Text);
}

void HSlideshow::nextPic() {
    if(!isVisible()) {
        s_sending=0;
        return;
    }
    if(!s_cur.size()||!s_cur.back()||!s_curPri.back()) {
        if(!s_curPri.size()) { s_curPri.push_back(0); s_cur.push_back(0); s_curTime.push_back(QTime()); }
        *(s_curPri.back()=s_artist.sendExtraPics(this,"addPic",s_i))=1;
        s_sending=0;
        return;
    }
    s_curTime.back()=QTime::currentTime();

    s_t=QTime::currentTime();
    ++s_i;
    if(s_i==9) s_i=0;

    s_curPri.push_back(0); s_cur.push_back(0); s_curTime.push_back(QTime());
    s_cur.back()=0;
    s_curPri.back()=s_artist.sendExtraPics(this,"addPic",s_i);
    *s_curPri.back()=1;
    s_sending=1;
    update();
}

void HSlideshow::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    for(int i=0;i<s_cur.size();i++) {

        if(!s_cur[i]||!s_curTime[i].isValid()) continue;

        double floatPer=1.0d-qMin(1.0d,double(s_curTime[i].msecsTo(QTime::currentTime()))/8000.0f);
        floatPer*=floatPer;
        floatPer=1.0d-floatPer;

        if(floatPer>0.999d) {
            for(int j=0;j<i;j++) {
                *s_curPri[0]=0;
                s_cur.pop_front();
                s_curPri.pop_front();
                s_curTime.pop_front();
            }
            i=0;
        }

        double centerX=double(width())/2.0d;
        double centerY=double(height())/2.0d;

        double dwidth=width();
        double dheight=height();
        double mwidth=qMin(dwidth,s_cur[i]->width()*(double(dheight)/double(s_cur[i]->height())));
        double mheight=qMin(dheight,s_cur[i]->height()*(double(dwidth)/double(s_cur[i]->width())));
        double xwidth=qMax(dwidth,s_cur[i]->width()*(double(dheight)/double(s_cur[i]->height())));
        double xheight=qMax(dheight,s_cur[i]->height()*(double(dwidth)/double(s_cur[i]->width())));

        dwidth=mwidth+(xwidth-mwidth)*floatPer;
        dheight=mheight+(xheight-mheight)*floatPer;

        p.setOpacity(qMin(1.0d,floatPer*2.0d));

        if(centerY-xheight/3<=0.0d) {
            p.drawImage(QRect( centerX-dwidth/2, centerY-dheight/(2.0f+floatPer), dwidth, dheight ),
                        *s_cur[i]);
        } else {
            p.drawImage(QRect( centerX-dwidth/2, centerY-dheight/2, dwidth, dheight ),
                        *s_cur[i]);
        }

    }
    QTimer::singleShot(20,this,SLOT(update()));
}

void HSlideshow::addPic(QImage& p) {
    if(!s_curPri.size()) { s_curPri.push_back(0); s_cur.push_back(0); s_curTime.push_back(QTime()); }
    s_cur.back()=&p;
    int a=s_t.msecsTo(QTime::currentTime());
    if(a>15000||s_t.isNull()) nextPic();
    else {
        QTimer::singleShot(15000-a,this,SLOT(nextPic()));
    }
}

void HSlideshow::addPic(QImage* p) {
    if(!p) {
        s_i=0;
        s_cur.back()=0;
        s_curPri.back()=0;
        nextPic();
    }
}

void HSlideshow::resume() {
    if(!s_sending) QTimer::singleShot(200,this,SLOT(nextPic()));
    s_sending=1;
}
