#ifndef HPLAYWIDGET_H
#define HPLAYWIDGET_H

#include "hartist.h"
#include <QWidget>

namespace Ui {
class HPlayWidget;
}

class HPlayWidget : public QWidget
{
    Q_OBJECT
    HObject* s_rep;
    enum State {
        SimilarArtists,
        MorphStation,
        TopAlbums,
        TopSongs
    } s_state;
    int s_echoSongCount;

public:
    explicit HPlayWidget(HArtist& artist, QWidget *parent = 0);
    explicit HPlayWidget(HTrack& artist, QWidget *parent = 0);
    explicit HPlayWidget(HAlbum& artist, QWidget *parent = 0);
    ~HPlayWidget();

public slots:
    void doSetup();
    void reset();
    void similarArtists();
    void morphStation();
    void topAlbums();
    void topSongs();

    void play1();
    void play2();
    void play3();
    void play4();
    void play5();

    void echo(HArtist*);
signals:
    void closed();
    
private:
    Ui::HPlayWidget *ui;
};

#endif // HPLAYWIDGET_H
