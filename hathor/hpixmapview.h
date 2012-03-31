#ifndef HPIXMAPVIEW_H
#define HPIXMAPVIEW_H

#include <QWidget>
#include <QImage>
#include "hfuture.h"

class HPixmapView : public QWidget
{
    Q_OBJECT
    QImage* s_p;
    int s_l;
public:
    HPixmapView(QWidget* parent=0,QImage* p=0) : QWidget(parent), s_p(p) {}
    void paintEvent(QPaintEvent *);
    QImage* pixmap() const { return s_p; }

    QSize sizeHint() const {
        if(isVisible()&&s_p) return s_p->size();
        else return QWidget::sizeHint();
    }

    QSize minimumSizeHint() const {
        if(isVisible()&&s_p) return s_p->size();
        else return QWidget::minimumSizeHint();
    }

    void showEvent(QShowEvent *e) {
        s_l=255;
        QWidget::showEvent(e);
    }

    void hideEvent(QHideEvent *e) {
        setPixmap(0);   //in case you forgot!
        Q_ASSERT(!s_p);
        QWidget::hideEvent(e);
    }

public slots:
    void setPixmap(QImage& p) { if(!isVisible()) return; s_l=255; s_p=&p; setMinimumSize(p.width(),p.height()); updateGeometry(); adjustSize(); update(); }
    void setPixmap(QImage* p) { if(!isVisible()&&p) return; s_l=255; s_p=p; if(p) { setMinimumSize(p->width(),p->height()); updateGeometry(); adjustSize(); update(); } }
};

#endif // HPIXMAPVIEW_H
