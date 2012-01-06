#ifndef HALBUM_H
#define HALBUM_H

#include "hobject.h"
#include <QStringList>
#include <QMap>
#include <QPixmap>
#include "htrack.h"
#include "htag.h"
#include "hshout.h"

class HArtist;

class HAlbum : public HObject
{
    QString s_artist, s_album;
    HAlbum(QString artist, QString album);
    static QMap<QString,HAlbum*> _map;
public:
    static HAlbum& get(QString artist,QString album);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    HArtist& getArtist();
    QString getArtistName() { return s_artist; }
    QString getAlbumName() { return s_album; }

    QList<HTrack*> getTracks();
    QStringList getTrackNames();

    int getPlayCount();
    int getListenerCount();
    int getUserPlayCount();
    QPixmap getPic(PictureSize size);
    QStringList getTagNames();
    QList<HTag*> getTags();
    QStringList getMoreTagNames();
    QList<HTag*> getMoreTags();
    QString getSummary();
    QString getContent();

    QList<HShout*> getShouts();

private:
    struct PictureData {
        bool got[4];
        QPixmap pics[4];
        PictureData() { for(int i=0;i<4;i++) got[i]=0; }
        void getData(QString url,PictureSize size);
    } s_pictureData;

    struct AlbumInfo {
        int playCount;
        int listenerCount;
        int userPlayCount;
        QString pics[4];
        QStringList tags;
        QStringList tracks;
        QString summary;
        QString content;
        bool got;
        AlbumInfo() : got(0) {}
        void getData(QString artist,QString album);
    } s_albumInfo;

    struct ExtraTagData {
        QStringList tags;
        bool got;
        ExtraTagData() : got(0) {}

        void getData(QString artist,QString album);
    } s_extraTagData;

    struct ShoutData {
        QList<HShout*> shouts;
        bool got;
        ShoutData() : got(0) {}
        void getData(QString artist, QString album);
    } s_shoutData;

    //Degenerate copy and assignment
    HAlbum(const HAlbum&);
    HAlbum& operator=(const HAlbum&);
};

#endif // HALBUM_H
