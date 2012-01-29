#ifndef HSTUPIDRESIZEWIDGET_H
#define HSTUPIDRESIZEWIDGET_H

#include <QScrollArea>
#include <QLabel>

class HStupidResizeWidget : public QScrollArea {
public:
    HStupidResizeWidget(QWidget*parent=0) : QScrollArea(parent) {}
    virtual QSize sizeHint() const {
        QSize ret=QScrollArea::sizeHint();
        QObjectList c=children();
        for(int i=0;i<c.size();i++) {
            if(dynamic_cast<QLabel*>(c[i])) {
                ret.setHeight(qMin(768,dynamic_cast<QLabel*>(c[i])->heightForWidth(dynamic_cast<QLabel*>(c[i])->width())+18));
                const_cast<HStupidResizeWidget*>(this)->setVerticalScrollBarPolicy((dynamic_cast<QLabel*>(c[i])->heightForWidth(dynamic_cast<QLabel*>(c[i])->width())+18<768)?
                                               Qt::ScrollBarAlwaysOff:Qt::ScrollBarAlwaysOn);
                return ret;
            }
            const QObjectList& b=c[i]->children();
            for(int j=0;j<b.size();j++) {
                c.push_back(b[j]);
            }
        }
        qFatal("HStupidResizeWidget should only be used on widgets with a label child!");
        return QSize(0,0);
    }
    virtual QSize minimumSizeHint() const { return sizeHint(); }
    void resizeEvent(QResizeEvent *e) {
        updateGeometry();
        QScrollArea::resizeEvent(e);
    }
};

#endif // HSTUPIDRESIZEWIDGET_H
