#ifndef HALBUMCONTEXT_H
#define HALBUMCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "halbum.h"

namespace Ui {
class HAlbumContext;
}

class HAlbumContext : public QWidget
{
    Q_OBJECT
    HAlbum& s_rep;
    int s_albumLoadCount, s_trackLoadCount, s_tagLoadCount, s_artistLoadCount,s_shoutLoadCount;
public:
    explicit HAlbumContext(HAlbum& rep, QWidget *parent = 0);
    ~HAlbumContext();

public slots:
    void showMoreBio();
    void loadTracks();
    void loadArtist();
    void loadTags();
    void loadShouts();

    void play();
    void playReplacing();

private:
    Ui::HAlbumContext *ui;
};

#endif // HALBUMCONTEXT_H
