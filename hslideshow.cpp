#include "hslideshow.h"
#include <QPropertyAnimation>

HSlideshow::HSlideshow(HTrack &track, QWidget *parent) : QGraphicsView(parent), s_track(track), s_i(0), s_z(0), s_done(0)
{
    setScene(&sc);
    setFixedWidth(700);
    setFixedHeight(500);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setBackgroundRole(QPalette::Text);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setSceneRect(0,0,700,500);
    QTimer::singleShot(300,this,SLOT(newPic()));
}

void HSlideshow::newPic() {
    if(s_done) {
        deleteLater();
        return;
    }
    if(s_i==s_track.getArtist().getExtraPicCount()||s_i>10) {
        s_i=0;
    }
    SlideshowItem* gpi=new SlideshowItem(s_track.getArtist().getExtraPic(s_i++).scaledToWidth(700,Qt::SmoothTransformation));
    sc.addItem(gpi);
    gpi->setTransformationMode(Qt::SmoothTransformation);
    gpi->setZValue(++s_z);

    QPropertyAnimation* pa=new QPropertyAnimation(gpi,"echoOpacity");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setEasingCurve(QEasingCurve::OutSine);
    pa->setDuration(3000);
    pa->start(QAbstractAnimation::DeleteWhenStopped);

    double startScale=qMin(1.0,500.0/gpi->boundingRect().height());
    double endScale=qMax(1.0,500.0/gpi->boundingRect().height());

    QPropertyAnimation* pa2=new QPropertyAnimation(gpi,"echoScale");
    pa2->setStartValue(startScale);
    pa2->setEndValue(endScale);
    pa2->setEasingCurve(QEasingCurve::InOutQuad);
    pa2->setDuration(7000);
    pa2->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa3=new QPropertyAnimation(gpi,"echoX");
    pa3->setStartValue((700.0-startScale*700.0)/2.0);
    pa3->setEndValue(0.0);
    pa3->setEasingCurve(QEasingCurve::InOutQuad);
    pa3->setDuration(7000);
    pa3->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa4=new QPropertyAnimation(gpi,"echoY");
    pa4->setStartValue(0.0);
    pa4->setEndValue((500.0-endScale*gpi->boundingRect().height())/3.0);
    pa4->setEasingCurve(QEasingCurve::InOutQuad);
    pa4->setDuration(7000);
    pa4->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(15000,this,SLOT(newPic()));
    QTimer::singleShot(30020,gpi,SLOT(deleteLater()));
}
