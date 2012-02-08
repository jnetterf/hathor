#ifndef TRACKCONTEXT_H
#define TRACKCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "hartist.h"
#include "hplaywidget.h"

namespace Ui {
class HTrackContext;
}

class QGraphicsBlurEffect;

class HTrackContext : public QWidget
{
    Q_OBJECT
    HTrack& s_rep;
    int s_albumLoadCount, s_artistLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount, s_similarToLoad;

    int s_listenerCountCache, s_playCountCache;
    int s_bpm;
    bool s_loved;
    QString s_character;
    QString s_key;
    bool s_contentSet;
    HPlayWidget* s_pw;
    QGraphicsBlurEffect* s_ge;
    explicit HTrackContext(HTrack& rep, QWidget *parent = 0);
    static QHash<QString,HTrackContext*> s_map;
    QWidget* s_slideshow;
    QTime s_showTime;
    QList<HTrack*> s_loadedSimilar;
    void showEvent(QShowEvent *e);
public:
    static HTrackContext* getContext(HTrack& rep);
    ~HTrackContext();

public slots:
    void showMoreBio();
    void setMePic(QPixmap p);
    void loadArtist();
    void loadAlbum();
//    void loadTracks();
    void loadTags();
    void loadSimilar(int s=-1);
    void loadShouts();
    void setSlideshow(QWidget*);

    void play();
    void hidePlay();

    void setContent(QString);
    void setListenerCount(int);
    void setPlayCount(int);
    void updateBoxes();
    void setUserPlayCount(int);

    void setAlbums(QList<HAlbum*>);
    void setTags(QList<HTag*>);
    void setShouts(QList<HShout*>);
    void setSimilar(HTrack*);

    void setLoved(bool);

    void setBpm(int);
    void setValence(double);
    void setAggression(double);
    void setAvgLoudness(double);
    void setPercussiveness(double);
    void setKey(int);
    void setEnergy(double);
    void setPunch(double);
    void setSoundCreativity(double);
    void setChordalClarity(double);
    void setTempoInstability(double);
    void setRhythmicIntricacy(double);
    void setSpeed(double);

    void toggleLoved();

private:
    Ui::HTrackContext *ui;
};

#endif // TRACKCONTEXT_H
