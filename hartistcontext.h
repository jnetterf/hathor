#ifndef ARTISTCONTEXT_H
#define ARTISTCONTEXT_H

#include <QWidget>
#include "hloginwidget.h"
#include "hartist.h"

namespace Ui {
class HArtistContext;
}

class HArtistContext : public QWidget
{
    Q_OBJECT
    HArtist& s_rep;
    int s_albumLoadCount, s_trackLoadCount, s_tagLoadCount, s_similarLoadCount,s_shoutLoadCount;
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
    void playMore();
    void playMoreReplacing();
    void add(int a=1, int b=5);
    void addMore();
    void addEvenMore();
    void addMost();
    void addMostReplacing();

    void evalShout();
    void sendShout();
    
private:
    Ui::HArtistContext *ui;
};

#endif // ARTISTCONTEXT_H
