#ifndef HALBUMBOX_H
#define HALBUMBOX_H

#include <QWidget>
#include "halbum.h"
#include "hgrowingwidget.h"

namespace Ui {
class HAlbumBox;
}

class HAlbumBox : public HGrowingWidget
{
    Q_OBJECT
    
    HAlbum& s_album;
public:
    explicit HAlbumBox(HAlbum& album, QWidget *parent = 0);
    ~HAlbumBox();

public slots:
    void requestContext() { emit contextRequested(s_album); }

signals:
    void contextRequested(HAlbum&);
    
private:
    Ui::HAlbumBox *ui;
};

#endif // HALBUMBOX_H
