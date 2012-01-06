#include "hslideshow.h"
#include <QPropertyAnimation>

HSlideshow::HSlideshow(HTrack &track, QWidget *parent) : QGraphicsView(parent), s_track(track), s_i(0)
{
    setScene(&sc);
    newPic();
    setFixedWidth(748);
    setFixedHeight(500);
    setBackgroundRole(QPalette::Text);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void HSlideshow::newPic() {
    if(s_i==s_track.getArtist().getExtraPicCount()||s_i>10) {
        s_i=0;
    }
    SlideshowItem* gpi=new SlideshowItem(s_track.getArtist().getExtraPic(s_i++).scaledToWidth(768,Qt::SmoothTransformation));
    sc.addItem(gpi);
    qDebug()<<gpi->x()<<gpi->y();
    gpi->setTransformationMode(Qt::SmoothTransformation);

    QPropertyAnimation* pa=new QPropertyAnimation(gpi,"echoOpacity");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setEasingCurve(QEasingCurve::InOutSine);
    pa->setDuration(5000);
    pa->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* pa2=new QPropertyAnimation(gpi,"echoScale");
    pa2->setStartValue(0.8);
    pa2->setEndValue(1.0);
    pa2->setEasingCurve(QEasingCurve::InOutSine);
    pa2->setDuration(5000);
    pa2->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(8000,this,SLOT(newPic()));
    QTimer::singleShot(20020,gpi,SLOT(deleteLater()));
}
