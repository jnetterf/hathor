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
    QGraphicsEffect* s_ge;
    HArtistPlayWidget* s_pw;
    int s_albumsToLoad, s_tracksToLoad, s_similarToLoad;
public:
    explicit HArtistContext(HArtist& rep, QWidget *parent = 0);
    ~HArtistContext();

public slots:
    void showMoreBio();
    void loadAlbums();
    void loadTracks();
    void loadTags();
    void loadSimilar();
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
