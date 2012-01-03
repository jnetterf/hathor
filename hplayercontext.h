#ifndef HPLAYERCONTEXT_H
#define HPLAYERCONTEXT_H

#include <QWidget>

namespace Ui {
class HPlayerContext;
}

class HTrack;
class HTrackContext;

class HPlayerContext : public QWidget
{
    Q_OBJECT
    static HPlayerContext* _singleton;
    explicit HPlayerContext(QWidget *parent = 0);
    HTrackContext* s_magic;
public:
    static HPlayerContext* singleton() { return _singleton=(_singleton?_singleton:new HPlayerContext); }
    ~HPlayerContext();

public slots:
    void showTrack(HTrack& t);
    
private:
    Ui::HPlayerContext *ui;
};

#endif // HPLAYERCONTEXT_H
