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

HSlideshow::HSlideshow(HArtist &artist, QWidget *parent) : QGraphicsView(parent), s_artist(artist), s_i(0), s_z(0), s_sending(0), s_t(), s_curPri(0), s_cur(0)
{
    setScene(&sc);
    setFixedWidth(900);
    setFixedHeight(600);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setBackgroundRole(QPalette::Text);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setSceneRect(0,0,900,590);
}

void HSlideshow::nextPic() {
    if(!isVisible()) {
        s_sending=0;
        return;
    }
    if(!s_cur||!s_curPri) {
        *(s_curPri=s_artist.sendExtraPics(this,"addPic",s_i))=1;
        s_sending=0;
        return;
    }

    SlideshowItem* gpi=new SlideshowItem(*s_cur,s_curPri);

    *s_curPri=0;
    HCachedPixmap::release();

    sc.addItem(gpi);
    gpi->setTransformationMode(Qt::SmoothTransformation);
    gpi->setZValue(++s_z);

    QPropertyAnimation* pa=new QPropertyAnimation(gpi,"echoOpacity");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setEasingCurve(QEasingCurve::OutSine);
    pa->setDuration(3000);
    pa->start(QAbstractAnimation::DeleteWhenStopped);

    double startScale=qMin(900.0/gpi->boundingRect().width(),600.0/gpi->boundingRect().height());
    double endScale=qMax(900.0/gpi->boundingRect().width(),600.0/gpi->boundingRect().height());

    QPropertyAnimation* pa2=new QPropertyAnimation(gpi,"echoScale");
    pa2->setStartValue(startScale);
    pa2->setEndValue(endScale);
    pa2->setEasingCurve(QEasingCurve::InOutQuad);
    pa2->setDuration(7000);
    pa2->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa3=new QPropertyAnimation(gpi,"echoX");
    pa3->setStartValue((900.0-startScale*gpi->boundingRect().width())/2.0);
    pa3->setEndValue(0.0);
    pa3->setEasingCurve(QEasingCurve::InOutQuad);
    pa3->setDuration(7000);
    pa3->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa4=new QPropertyAnimation(gpi,"echoY");
    pa4->setStartValue(0.0);
    pa4->setEndValue((600.0-endScale*gpi->boundingRect().height())/3.0);
    pa4->setEasingCurve(QEasingCurve::InOutQuad);
    pa4->setDuration(7000);
    pa4->start(QAbstractAnimation::DeleteWhenStopped);

    s_t=QTime::currentTime();
    QTimer::singleShot(38000,gpi,SLOT(deleteLater()));
    ++s_i;
    if(s_i==9) s_i=0;
    s_cur=0;
    s_curPri=s_artist.sendExtraPics(this,"addPic",s_i);
    *s_curPri=1;
    s_sending=1;
}

void HSlideshow::addPic(QImage& p) {
//    if(p.width()>900) p=p.scaledToWidth(900,Qt::SmoothTransformation);
    s_cur=&p;
    int a=s_t.msecsTo(QTime::currentTime());
    if(a>15000||s_t.isNull()) nextPic();
    else {
        QTimer::singleShot(15000-a,this,SLOT(nextPic()));
    }
}

void HSlideshow::addPic(QImage* p) {
    if(!p) {
        s_i=0;
        s_cur=0;
        s_curPri=0;
        nextPic();
    }
}

void HSlideshow::resume() {
    if(!s_sending) QTimer::singleShot(200,this,SLOT(nextPic()));
    s_sending=1;
}
