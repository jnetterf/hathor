#ifndef HTRACKBOX_H
#define HTRACKBOX_H

#include "htrack.h"
#include "hgrowingwidget.h"

#include <QWidget>

namespace Ui {
class HTrackBox;
}

class HTrackBox : public HGrowingWidget
{
    Q_OBJECT
    HTrack& s_ref;
    int s_playCountCache;
    int s_userPlayCountCache;
    bool s_lovedCache;
    QString s_tags;
    int s_score;
    explicit HTrackBox(HTrack& ref, QWidget *parent = 0);
    QList<int**> s_priority[4];
public:
    static HTrackBox* getBox(HTrack& ref);
    ~HTrackBox();

    void showEvent(QShowEvent *e) { readjustPriorities(); QWidget::showEvent(e); }
    void hideEvent(QHideEvent *e) { readjustPriorities(); QWidget::hideEvent(e); }

    void readjustPriorities() {
        const static int a[4] = {65,50,40,30};
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

public slots:
    void requestContext() { emit contextRequested(s_ref); }

    void setPlayCount(int);
    void setUserPlayCount(int);
    void setLoved(bool);
    void setTags(QStringList);

    void updateBoxes();

signals:
    void contextRequested(HTrack&);

private:
    Ui::HTrackBox *ui;
};

#endif // HTRACKBOX_H
