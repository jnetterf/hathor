#ifndef HGROWINGWIDGET_H
#define HGROWINGWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QDebug>

class HGrowingWidget : public QWidget
{
public:
    HGrowingWidget(QWidget* parent=0);

    void resizeEvent(QResizeEvent *ev) {
        setMinimumHeight(qMax(height(),ev->oldSize().height()));
        if(ev->oldSize().height()>height()) {
        }

    }
};

#endif // HGROWINGWIDGET_H
