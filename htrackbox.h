#ifndef HTRACKBOX_H
#define HTRACKBOX_H

#include "htrack.h"

#include <QWidget>

namespace Ui {
class HTrackBox;
}

class HTrackBox : public QWidget
{
    Q_OBJECT
    HTrack& s_ref;
public:
    explicit HTrackBox(HTrack& ref, QWidget *parent = 0);
    ~HTrackBox();

public slots:
    void requestContext() { emit contextRequested(s_ref); }

signals:
    void contextRequested(HTrack&);

private:
    Ui::HTrackBox *ui;
};

#endif // HTRACKBOX_H
