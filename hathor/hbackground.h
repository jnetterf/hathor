#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QDebug>
#include <QTime>
#include "hartist.h"
#include <lastfm/Artist>

class ArtistAvatar;

class HMainContextOptions;

class HBackground : public QObject {
    Q_OBJECT
    HMainContextOptions* s_mco;
    enum Mode {
        Top=0,
        Suggestions,
        Local
    } s_mode;

    enum Style {
        Album=0,
        List,
        Play
    } s_style;

    // CACHE {
    QList<lastfm::Artist> list;
    int w,x;
    QList<int> _nv;
    QList<int> _nX;

    QList<ArtistAvatar*> x_;
    QList<ArtistAvatar*> l_[20];

    QTime curtime;
    int l;
    int maxY;

    int s_drawingI;
    //}

    QList<QGraphicsItem*> s_cache;
    QList<lastfm::Artist> s_toplist;
    bool s_gotTop, s_gotRec;
    QList<lastfm::Artist> s_reclist;

    bool s_stopRequest;
    bool s_showingStuff;

public:
    HBackground(QGraphicsScene*sc);
public slots:
    void showContext();
    void showStuff();
    void showStuff_makeList();
    void continueShowStuff();
    void showStuff_addPic(QPixmap);

    void doStopRequest();
    void onSuggMode();
    void onLocalMode();
    void onTopMode();

    void onAlbumMode();
    void onListMode();
    void onPlayMode();
private:
    QGraphicsScene* _sc;
signals:
    void showContext(HArtist&);
};

#endif // BACKGROUND_H
