#ifndef HALBUMCONTEXT_H
#define HALBUMCONTEXT_H

#include <QWidget>
#include <QGraphicsBlurEffect>
#include "hloginwidget.h"
#include "halbum.h"
#include "hplaywidget.h"

namespace Ui {
class HAlbumContext;
}

class HAlbumContext : public QWidget
{
    Q_OBJECT
    HAlbum& s_rep;
    int s_albumLoadCount, s_trackLoadCount, s_tagLoadCount, s_artistLoadCount,s_shoutLoadCount;
    int s_cachedPlayCount,s_cachedListenerCount,s_cachedUserPlayCount;
    int deltaWidth;
    QTime s_showTime;
    HPlayWidget* s_pw;
    QGraphicsBlurEffect* s_ge;
    explicit HAlbumContext(HAlbum& rep, QWidget *parent = 0);
    static QHash<QString,HAlbumContext*> s_map;
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

    QList<int**> s_priority[4];
public:
    static HAlbumContext* getContext(HAlbum& rep);
    ~HAlbumContext();

public slots:
    void showMoreBio();
    void loadTracks();
    void loadTracks_2(HTrack *track);
    void loadArtist();
    void loadTags();
    void loadShouts();

    void play();
    void hidePlay();

    void setPlayCount(int);
    void setListenerCount(int);
    void setUserPlayCount(int);
    void updateBoxes();

    void setPic(QPixmap &);
    void setSummary(QString);
    void setMePic(QPixmap &p);
    void setShouts(QList<HShout*>);
    void addTags(QList<HTag*> tags);

    void readjustPriorities() {
        const static int a[4] = {90,80,70,60};
        for(int i=3;i>=0;--i) {
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

private:
    Ui::HAlbumContext *ui;
};

#endif // HALBUMCONTEXT_H
