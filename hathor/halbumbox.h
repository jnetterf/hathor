#ifndef HALBUMBOX_H
#define HALBUMBOX_H

#include <QWidget>
#include <QTime>
#include "halbum.h"
#include "hgrowingwidget.h"

namespace Ui {
class HAlbumBox;
}

class HAlbumBox : public HGrowingWidget
{
    Q_OBJECT
    
    HAlbum& s_album;
    int s_cachedPlayCount,s_cachedListenerCount,s_cachedUserPlayCount;
    explicit HAlbumBox(HAlbum& album, QWidget *parent = 0);
    static QHash<QString,HAlbumBox*> s_map;
    QTime s_showTime;
    QString s_tagString;
    bool s_gotTags;

public:
    static HAlbumBox* getBox(HAlbum& album);
    ~HAlbumBox();

    void showEvent(QShowEvent *e) { s_showTime=QTime::currentTime(); QWidget::showEvent(e); }

public slots:
    void requestContext() { emit contextRequested(s_album); }
    void setPlayCount(int count);
    void setListenerCount(int count);
    void setUserPlayCount(int count);
    void updateCounts();

    void setPixmap(QPixmap);
    void setTagNames(QStringList);

signals:
    void contextRequested(HAlbum&);
    
private:
    Ui::HAlbumBox *ui;
};

#endif // HALBUMBOX_H
