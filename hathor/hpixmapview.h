#ifndef HPIXMAPVIEW_H
#define HPIXMAPVIEW_H

#include <QWidget>
#include <QPixmap>
#include "hfuture.h"

class HPixmapView : public QWidget
{
    Q_OBJECT
    QPixmap* s_p;
public:
    HPixmapView(QWidget* parent=0,QPixmap* p=0) : QWidget(parent), s_p(p) {}
    void paintEvent(QPaintEvent *);
    QPixmap* pixmap() const { return s_p; }

    QSize sizeHint() const {
        if(isVisible()&&s_p) return s_p->size();
        else return QWidget::sizeHint();
    }

    QSize minimumSizeHint() const {
        if(isVisible()&&s_p) return s_p->size();
        else return QWidget::minimumSizeHint();
    }

    void hideEvent(QHideEvent *e) {
        setPixmap(0);   //in case you forgot!
        QWidget::hideEvent(e);
    }

public slots:
    void setPixmap(QPixmap& p) { if(isHidden()) return; s_p=&p; setMinimumSize(p.width(),p.height()); updateGeometry(); adjustSize(); update(); }
    void setPixmap(QPixmap* p) { if(isHidden()) return; s_p=p; if(p) { setMinimumSize(p->width(),p->height()); updateGeometry(); adjustSize(); update(); } }
};

#endif // HPIXMAPVIEW_H
