#ifndef HPLAYERCONTEXT_H
#define HPLAYERCONTEXT_H

#include <QWidget>

namespace Ui {
class HPlayerContext;
}

class HTrack;
class HTrackContext;
class HSlideshow;

class HPlayerContext : public QWidget
{
    Q_OBJECT
    static HPlayerContext* _singleton;
    explicit HPlayerContext(QWidget *parent = 0);
    HTrackContext* s_magic;
    HSlideshow* s_slideshow;
public:
    static HPlayerContext* singleton() { return _singleton=(_singleton?_singleton:new HPlayerContext); }
    ~HPlayerContext();

    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

public slots:
    void showTrack(HTrack& t);
    
private:
    Ui::HPlayerContext *ui;
};

#endif // HPLAYERCONTEXT_H
