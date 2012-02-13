#ifndef ARTISTCONTEXT_H
#define ARTISTCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "hartist.h"

namespace Ui {
class HArtistContext;
}

class HPlayWidget;

class HArtistContext : public QWidget
{
    Q_OBJECT
    HArtist& s_rep;
    int s_albumLoadCount, s_trackLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount;
    int s_playCountCache, s_listenerCountCache, s_userPlayCountCache, deltaWidth;
    int s_tagsToLoad;
    QGraphicsEffect* s_ge;
    HPlayWidget* s_pw;
    int s_albumsToLoad, s_tracksToLoad, s_similarToLoad;

    void resizeEvent(QResizeEvent *);
    QList<HAlbum*> s_loadedAlbums;
    QList<HTrack*> s_loadedTracks;
    QList<HArtist*> s_loadedSimilar;

    explicit HArtistContext(HArtist& rep, QWidget *parent = 0);
    static QHash<QString,HArtistContext*> s_map;

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    QTime s_showTime;

    QList<int**> s_priority[4];

    void readjustPriorities() {
        const static int a[4] = {90,80,70,60};
        for(int i=4;i>=0;--i) {
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

public:
    static HArtistContext* getContext(HArtist& rep);
    ~HArtistContext();

public slots:
    void continueLoading();
    void showMoreBio();
    void loadAlbums(int s=-1);
    void loadTracks(int s=-1);
    void loadTags(int s=-1);
    void loadSimilar(int s=-1);
    void loadShouts();

    void play();
    void hidePlay();

    void evalShout();
    void sendShout();
    void onShoutSent();
    void setPic(QPixmap &p);
    void setBio(QString bio);
    void setMePic(QPixmap &p);

    void setPlayCount(int a);
    void setListenerCount(int a);
    void setUserPlayCount(int a);
    void updateCounts();

    void setAlbums(HAlbum*);
    void setTracks(HTrack*);
    void setTags(QList<HTag*>);
    void setSimilar(HArtist* similar);
    void setShouts(QList<HShout*> shouts);
    
private:
    Ui::HArtistContext *ui;
};

#endif // ARTISTCONTEXT_H
