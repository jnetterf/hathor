#include "hpixmapview.h"
#include <QPainter>
#include <QDebug>

void HPixmapView::paintEvent(QPaintEvent *e) {
    if(isHidden()) {
        if(s_p) s_p=0;
        return;
    }
    QPainter painter(this);
    if(s_p&&!s_p->isNull()) painter.drawPixmap(0,0,s_p->width(),s_p->height(),*s_p);
    QWidget::paintEvent(e);
}
