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
public:
    explicit HTrackContext(HTrack& rep, QWidget *parent = 0);
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
    void playReplacing();
    void playSimilar();
    void playMoreSimilar();
    void playMoreSimilarReplacing();


private:
    Ui::HTrackContext *ui;
};

#endif // TRACKCONTEXT_H
