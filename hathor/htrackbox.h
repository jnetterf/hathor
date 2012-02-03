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
    static QHash<QString,HTrackBox*> s_map;
    explicit HTrackBox(HTrack& ref, QWidget *parent = 0);
public:
    static HTrackBox* getBox(HTrack& ref);
    ~HTrackBox();

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
