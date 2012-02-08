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

HSlideshow::HSlideshow(HArtist &artist, QWidget *parent) : QGraphicsView(parent), s_artist(artist), s_i(0), s_z(0), s_done(0)
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
    setSceneRect(0,0,900,600);
    s_artist.sendExtraPics(this,"addPic",15);
}

void HSlideshow::nextPic() {
    if(s_done) {
        return;
    }
    Q_ASSERT(s_cache.size());
    if(!s_cache.size()) return;
    if(s_i>=s_cache.size()) {
        s_i=0;
    }

    SlideshowItem* gpi=new SlideshowItem(s_cache[s_i].scaledToWidth(900,Qt::SmoothTransformation));
    sc.addItem(gpi);
    gpi->setTransformationMode(Qt::SmoothTransformation);
    gpi->setZValue(++s_z);

    QPropertyAnimation* pa=new QPropertyAnimation(gpi,"echoOpacity");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setEasingCurve(QEasingCurve::OutSine);
    pa->setDuration(3000);
    pa->start(QAbstractAnimation::DeleteWhenStopped);

    double startScale=qMin(1.0,600.0/gpi->boundingRect().height());
    double endScale=qMax(1.0,600.0/gpi->boundingRect().height());

    QPropertyAnimation* pa2=new QPropertyAnimation(gpi,"echoScale");
    pa2->setStartValue(startScale);
    pa2->setEndValue(endScale);
    pa2->setEasingCurve(QEasingCurve::InOutQuad);
    pa2->setDuration(7000);
    pa2->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa3=new QPropertyAnimation(gpi,"echoX");
    pa3->setStartValue((900.0-startScale*900.0)/2.0);
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

    QTimer::singleShot(15000,this,SLOT(nextPic()));
    QTimer::singleShot(30020,gpi,SLOT(deleteLater()));
}

void HSlideshow::addPic(QPixmap p) {
    s_cache.push_back(p);
    if(s_cache.size()==1) nextPic();
}
