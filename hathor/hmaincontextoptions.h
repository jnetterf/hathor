#ifndef HMAINCONTEXTOPTIONS_H
#define HMAINCONTEXTOPTIONS_H

#include <QWidget>
#include <QDebug>

namespace Ui {
class HMainContextOptions;
}

class HMainContextOptions : public QWidget
{
    Q_OBJECT
    
public:
    explicit HMainContextOptions(QWidget *parent = 0);
    ~HMainContextOptions();

public slots:
    void updateIcons();

    void onAllArtistsToggled(bool a) {
        if(a) emit topMode();
    }

    void onSuggestionsToggled(bool a) {
        if(a) emit suggMode();
    }

    void onLocalToggled(bool a) {
        if(a) emit localMode();
    }




    void onAlbumModeToggled(bool a) {
        if(a) emit albumMode();
    }

    void onListModeToggled(bool a) {
        if(a) emit listMode();
    }

    void onPlayModeToggled(bool a) {
        if(a) emit playMode();
    }

signals:
    void topMode();
    void suggMode();
    void localMode();

    void albumMode();
    void listMode();
    void playMode();
    
private:
    Ui::HMainContextOptions *ui;
};

#endif // HMAINCONTEXTOPTIONS_H
