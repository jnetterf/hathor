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
    QTime s_showTime;
    QString s_tagString;
    bool s_gotTags;

    QList<int**> s_priority[4];

public:
    static HAlbumBox* getBox(HAlbum& album);
    ~HAlbumBox();

    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

    void readjustPriorities() {
        const static int a[4] = {65,50,40,30};
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

public slots:
    void requestContext() { emit contextRequested(s_album); }
    void setPlayCount(int count);
    void setListenerCount(int count);
    void setUserPlayCount(int count);
    void updateCounts();

    void setPixmap(QPixmap&);
    void setTagNames(QStringList);

signals:
    void contextRequested(HAlbum&);
    
private:
    Ui::HAlbumBox *ui;
};

#endif // HALBUMBOX_H
