#ifndef HPLAYWIDGET_H
#define HPLAYWIDGET_H

#include "hartist.h"
#include <QWidget>

namespace Ui {
class HPlayWidget;
}

class HArtistPlayWidget : public QWidget
{
    Q_OBJECT
    HArtist& s_artist;
    enum State {
        SimilarArtists,
        MorphStation,
        TopAlbums,
        TopSongs
    } s_state;
    int s_echoSongCount;

public:
    explicit HArtistPlayWidget(HArtist& artist, QWidget *parent = 0);
    ~HArtistPlayWidget();

public slots:
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
