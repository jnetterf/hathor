#ifndef TRACKCONTEXT_H
#define TRACKCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "hartist.h"

namespace Ui {
class HTrackContext;
}

class HTrackContext : public QWidget
{
    Q_OBJECT
    HTrack& s_rep;
    int s_albumLoadCount, s_artistLoadCount, s_trackLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount;

    int s_listenerCountCache, s_playCountCache;
    int s_bpm;
    QString s_character;
    QString s_key;
    bool s_contentSet;
    explicit HTrackContext(HTrack& rep, QWidget *parent = 0);
    static QHash<QString,HTrackContext*> s_map;
    QWidget* s_slideshow;
public:
    static HTrackContext* getContext(HTrack& rep);
    ~HTrackContext();

public slots:
    void showMoreBio();
    void loadArtist();
    void loadAlbum();
//    void loadTracks();
    void loadTags();
    void loadSimilar();
    void loadShouts();
    void setSlideshow(QWidget*);

    void playTrack();

    void setContent(QString);
    void setListenerCount(int);
    void setPlayCount(int);
    void updateBoxes();
    void setUserPlayCount(int);

    void setAlbums(QList<HAlbum*>);
    void setTags(QList<HTag*>);
    void setShouts(QList<HShout*>);
    void setSimilar(QList<HTrack*>);

    void setBpm(double);
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

private:
    Ui::HTrackContext *ui;
};

#endif // TRACKCONTEXT_H
