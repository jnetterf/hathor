#ifndef HSLIDESHOW_H
#define HSLIDESHOW_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include "htrack.h"
#include "hartist.h"

class HSlideshow : public QWidget
{
    HTrack& s_track;
public:
    explicit HSlideshow(HTrack& track,QWidget* parent=0);
};

#endif // HSLIDESHOW_H
