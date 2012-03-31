#include "hpixmapview.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>

void HPixmapView::paintEvent(QPaintEvent *e) {
    if(!isVisible()) {
        if(s_p) s_p=0;
        return;
    }
    QPainter painter(this);
    if(s_p) painter.drawImage(QRect(0,0,s_p->width(),s_p->height()),*s_p);
    if(s_l>0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(255,255,255,s_l)));
        painter.drawRect(0,0,width(),height());
        s_l-=10;
        QTimer::singleShot(20,this,SLOT(update()));
    }
    QWidget::paintEvent(e);
}
