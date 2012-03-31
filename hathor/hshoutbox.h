#ifndef HSHOUTBOX_H
#define HSHOUTBOX_H

#include <QWidget>
#include "hshout.h"
#include "hgrowingwidget.h"

namespace Ui {
class HShoutBox;
}

class HShoutBox : public HGrowingWidget
{
    Q_OBJECT
    
    HShout& s_rep;
public:
    explicit HShoutBox(HShout& rep, QWidget *parent);
    ~HShoutBox();

    QList<int**> s_priority[4];

    void readjustPriorities() {
        const static int a[4] = {90,80,70,60};
        for(int i=3;i>=0;--i) {
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

    void showEvent(QShowEvent *e) {
        s_priority[0].push_back(s_rep.getShouter().sendPic(HUser::Medium,this,"setPic"));
        readjustPriorities();
        QWidget::showEvent(e);
    }

    void hideEvent(QHideEvent *e) {
        readjustPriorities();
        QWidget::hideEvent(e);
    }

public slots:
    void setPic(QImage &p);

private:
    Ui::HShoutBox *ui;
};


#endif // HSHOUTBOX_H
