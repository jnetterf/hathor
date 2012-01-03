#ifndef HGRAPHICSVIEW_H
#define HGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPropertyAnimation>

class HGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit HGraphicsView(QWidget *parent = 0);
    int s_hidePos;
    int s_max;
    
protected:
    void wheelEvent(QWheelEvent *event) {
        QPropertyAnimation* pa=new QPropertyAnimation(verticalScrollBar(),"value");
        pa->setStartValue(verticalScrollBar()->value());
        pa->setEndValue(verticalScrollBar()->value()-event->delta()*2.5);
        pa->setDuration(100);
        pa->start(QPropertyAnimation::DeleteWhenStopped);
    }
    void resizeEvent(QResizeEvent *event) {
        QGraphicsView::resizeEvent(event);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        verticalScrollBar()->setMaximum(s_max);
    }
    void showEvent(QShowEvent *event) {
        QGraphicsView::showEvent(event);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        verticalScrollBar()->setMaximum(s_max);
        verticalScrollBar()->setValue(s_hidePos);
    }
    void hideEvent(QHideEvent *event) {
        s_hidePos=verticalScrollBar()->value();
        QGraphicsView::hideEvent(event);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        verticalScrollBar()->setMaximum(s_max);
    }

signals:
    
public slots:
    void setMax(int max) {
        s_max=max;
        verticalScrollBar()->setMaximum(s_max);
    }
    
};

#endif // HGRAPHICSVIEW_H
