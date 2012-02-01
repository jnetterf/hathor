#ifndef HTOOLBAR_H
#define HTOOLBAR_H

#include <QWidget>
#include "htrack.h"
#include "habstractmusicinterface.h"

namespace Ui {
class HToolbar;
}

class HToolbar : public QWidget
{
    Q_OBJECT
    QString s_message;
    QString s_playback;
    static HToolbar* _singleton;

public:
    static HToolbar* singleton() { return _singleton; }
    explicit HToolbar(QWidget *parent = 0);
    ~HToolbar();

    QString message();

public slots:
    void setMessage(QString);
    void setMessageSimple(QString);
    void clearMessage();

    void setMessage2();
    void clearMessage2();

    void setPlaybackStatus(QString s);

    void setBackEnabled(bool enabled);
    void setPlayEnabled(bool enabled);
    void setPlayChecked(bool checked);
    bool playChecked();

    void tryPlay(bool play);
    void tryNext();

    void onStateChanged(HAbstractTrackInterface::State);
    void onTrackChanged(HTrack&);

signals:
    void shuffleToggled(bool);
    
private:
    Ui::HToolbar *ui;
};

#endif // HTOOLBAR_H
