#ifndef HALBUM_H
#define HALBUM_H

#include "hobject.h"
#include <QStringList>
#include <QMap>
#include <QPixmap>
#include <QMutex>
#include "htrack.h"
#include "htag.h"
#include "hshout.h"
#include "hfuture.h"
#include "libhathor_global.h"

class HArtist;

class HAlbum;

class LIBHATHORSHARED_EXPORT AlbumInfo : public HCachedInfo {
    friend class HAlbum;
    AlbumInfo(QString artist, QString album);
    bool process(const QString& data);
};

class LIBHATHORSHARED_EXPORT ExtraTagData : public HCachedInfo {
    friend class HAlbum;
    ExtraTagData(QString artist, QString album);
    bool process(const QString& data);
};

struct LIBHATHORSHARED_EXPORT ShoutData : public QObject {
    Q_OBJECT
    friend class HAlbum;
private:
    QMutex mutex, mutex_2;
    QString artist, album;
    QList<HShout*> shouts;  QList< QPair<QObject*,QString> > shoutQueue;
    bool got;
    HRunOnceNotifier* getting;
    ShoutData() : got(0), getting(0) {}
public slots:
    void sendData(QString artist, QString album);
    void sendData_process();
    void sendData_processQueue();
};

class LIBHATHORSHARED_EXPORT HAlbum : public HObject
{
    Q_OBJECT
    QMutex queueMutex;
    QString s_artist, s_album;
    HAlbum(QString artist, QString album);
    static QHash<QString,HAlbum*> _map;
    QList< QPair<QObject*,QString> > s_trackQueue;
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_moreTagQueue;
    QList< QPair<QObject*,QString> > s_picQueue[4];
    HCachedPixmap* s_cachedPixmap[4];
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

    bool isCached() { return s_albumInfo.isCached(); }

public slots:
    void sendTracks(QObject* obj, const char* member);  /*QList<HTrack*>*/
    void sendTracks_2(QStringList);  /*for internal use*/
    void sendTrackNames(QObject* obj, const char* member); /*QStringList*/
    void sendPlayCount(QObject* obj, const char* member); /*int*/
    void sendListenerCount(QObject* obj, const char* member); /*int*/
    void sendUserPlayCount(QObject* obj, const char* member); /*int*/

    void sendPicNames(HAlbum::PictureSize size,QObject* obj, const char* member); /*QString*/
    void sendPic(HAlbum::PictureSize size,QObject* obj, const char* member); /*QPixmap*/
    void sendPic_2_0(QString pic) { sendPic_2((PictureSize)0,pic); }
    void sendPic_2_1(QString pic) { sendPic_2((PictureSize)1,pic); }
    void sendPic_2_2(QString pic) { sendPic_2((PictureSize)2,pic); }
    void sendPic_2_3(QString pic) { sendPic_2((PictureSize)3,pic); }
    void sendPic_2(HAlbum::PictureSize p,QString pic); /*for internal use*/
    void sendTagNames(QObject* obj, const char* member); /*QStringList*/
    void sendTags(QObject* obj, const char* member); /*QList<HTag*>*/
    void sendTags_2(QStringList); /*for internal use*/
    void sendMoreTagNames(QObject* obj, const char* member);/*QStringList*/
    void sendMoreTags(QObject* obj, const char* member); /*QList<HTag*>*/
    void sendMoreTags_2(QStringList); /*for internal use*/
    void sendSummary(QObject* obj, const char* member); /*QString*/
    void sendContent(QObject* obj, const char* member); /*QList<HTag*>*/
    void sendShouts(QObject* obj, const char* member); /*QList<HShout*>*/
public:
//    QPixmap getPic(PictureSize size);
//    QStringList getTagNames();
//    QList<HTag*> getTags();
//    QStringList getMoreTagNames();
//    QList<HTag*> getMoreTags();
//    QString getSummary();
//    QString getContent();

//    QList<HShout*> getShouts();

private:
    AlbumInfo s_albumInfo;
    ExtraTagData s_extraTagData;
    ShoutData s_shoutData;

    //Degenerate copy and assignment
    HAlbum(const HAlbum&);
    HAlbum& operator=(const HAlbum&);
};

#endif // HALBUM_H
