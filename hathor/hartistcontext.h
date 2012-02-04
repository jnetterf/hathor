#ifndef ARTISTCONTEXT_H
#define ARTISTCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "hartist.h"

namespace Ui {
class HArtistContext;
}

class HArtistPlayWidget;

class HArtistContext : public QWidget
{
    Q_OBJECT
    HArtist& s_rep;
    int s_albumLoadCount, s_trackLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount;
    int s_playCountCache, s_listenerCountCache, s_userPlayCountCache, deltaWidth;
    int s_tagsToLoad;
    QGraphicsEffect* s_ge;
    HArtistPlayWidget* s_pw;
    int s_albumsToLoad, s_tracksToLoad, s_similarToLoad;

    void resizeEvent(QResizeEvent *);
    QList<HAlbum*> s_loadedAlbums;
    QList<HTrack*> s_loadedTracks;
    QList<HArtist*> s_loadedSimilar;

    explicit HArtistContext(HArtist& rep, QWidget *parent = 0);
    static QHash<QString,HArtistContext*> s_map;

    void showEvent(QShowEvent *);
    QTime s_showTime;
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
    void setPic(QPixmap p);
    void setBio(QString bio);

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
