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
    friend class HPlayerContext;
    HTrack& s_rep;
    int s_albumLoadCount, s_artistLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount;
    int s_similarToLoad,s_shoutsToLoad;

    int s_listenerCountCache, s_playCountCache;
    int s_bpm;
    bool s_loved;
    QString s_character;
    QString s_key;
    bool s_contentSet;
    HPlayWidget* s_pw;
    QList<HShout*> s_loadedShouts;
    QGraphicsBlurEffect* s_ge;
    explicit HTrackContext(HTrack& rep, QWidget *parent = 0);
    QWidget* s_slideshow;
    QTime s_showTime;
    QList<HTrack*> s_loadedSimilar;
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    QList<int**> s_priorities[4];
public:
    static HTrackContext* getContext(HTrack& rep);
    HTrack& getTrack() { return s_rep; }
    ~HTrackContext();

public slots:
    void showMoreBio();
    void setMePic(QImage &p);
    void loadArtist();
    void loadAlbum();
//    void loadTracks();
    void loadTags();
    void loadSimilar(int s=-1);
    void loadShouts(int s=-1);
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
    void setShouts(HShout*);
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
    void evalShout();
    void sendShout();

    void toggleLoved();

    void readjustPriorities() {
        const static int a[4] = {90,80,70,60};
        for(int i=3;i>=0;--i) {
            for(int j=0;j<s_priorities[i].size();j++) {
                if(s_priorities[i][j]) {
                    if(!*s_priorities[i][j]) *s_priorities[i][j]=new int;
                    if(isVisible()) {
                        **s_priorities[i][j]=a[j];
                    } else {
                        **s_priorities[i][j]=0;
                    }
                }
            }
        }
    }

    void setLyrics(QString);

private:
    Ui::HTrackContext *ui;
};

#endif // TRACKCONTEXT_H
