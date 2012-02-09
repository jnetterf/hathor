#ifndef HARTISTBOX_H
#define HARTISTBOX_H

#include <QWidget>
#include <QTime>
#include "hartist.h"
#include "hgrowingwidget.h"

namespace Ui {
class HArtistBox;
}

class HArtistBox : public HGrowingWidget
{
    Q_OBJECT
    HArtist& s_rep;

    explicit HArtistBox(HArtist& rep, QWidget *parent = 0);
    static QHash<QString,HArtistBox*> s_map;
    QTime s_showTime;

    QList<int**> s_priority[4];
    void showEvent(QShowEvent *e) { s_showTime=QTime::currentTime(); QWidget::showEvent(e); readjustPriorities(); }
    void hideEvent(QHideEvent *e) { QWidget::hideEvent(e); readjustPriorities(); }
public:
    static HArtistBox* getBox(HArtist& rep);
    ~HArtistBox();

    void readjustPriorities() {
        const static int a[4] = {65,50,40,30};
        for(int i=4;i>=0;--i) {
            for(int j=0;j<s_priority[i].size();j++) {
                if(s_priority[i][j]) {
                    if(!*s_priority[i][j]) *s_priority[i][j]=new int;
                    if(isVisible()) {
                        **s_priority[i][j]=a[j];
                    } else {
                        **s_priority[i][j]=0;
                    }
                }
            }
        }
    }
    
public slots:
//    void continueLoading();
    void requestContext() { emit contextRequested(s_rep); }
    void setPic(QPixmap p);
    void setTagNames(QStringList tsl);
    void setBioShort(QString sh);
signals:
    void contextRequested(HArtist& rep);

private:
    Ui::HArtistBox *ui;
};

#endif // HARTISTBOX_H
