#ifndef HALBUM_H
#define HALBUM_H

#include "hobject.h"
#include <QStringList>
#include <QMap>
#include <QImage>
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

class LIBHATHORSHARED_EXPORT HAlbum : public HObject
{
    Q_OBJECT
    QMutex queueMutex;
    QString s_artist, s_album;
    HAlbum(QString artist, QString album);
    static QHash<QString,HAlbum*> _map;
    QList<HShout*> shouts;
    QList< QPair<QObject*,QString> > s_trackQueue;
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_moreTagQueue;
    QList< QPair<QObject*,QString> > s_picQueue[4];
    struct HTriple {
        QObject* first;
        QString second;
        int third;
        HTriple(QObject* a,const QString& b,const int& c) : first(a),second(b),third(c) {}
    };
    QList< HTriple > s_shoutQueue;
    HCachedPixmap* s_cachedPixmap[4];
    QStringList s_shout_shouts,s_shout_artists,s_shout_dates;
    bool s_gotShouts;
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
    int** sendTracks(QObject* obj, const char* member);  /*many HTrack* ending in 0 */
    int** sendTrackNames(QObject* obj, const char* member,QObject* guest=0); /*QStringList*/
    int** sendPlayCount(QObject* obj, const char* member); /*int*/
    int** sendListenerCount(QObject* obj, const char* member); /*int*/
    int** sendUserPlayCount(QObject* obj, const char* member); /*int*/

    int** sendPicNames(HAlbum::PictureSize size,QObject* obj, const char* member,QObject* guest=0); /*QString*/
    int** sendPic(HAlbum::PictureSize size,QObject* obj, const char* member); /*QImage&*/
    int** sendTagNames(QObject* obj, const char* member); /*QStringList*/
    int** sendTags(QObject* obj, const char* member); /*QList<HTag*>*/
    int** sendMoreTagNames(QObject* obj, const char* member, QObject *guest);/*QStringList*/
    int** sendMoreTags(QObject* obj, const char* member); /*QList<HTag*>*/
    int** sendSummary(QObject* obj, const char* member); /*QString*/
    int** sendContent(QObject* obj, const char* member); /*QList<HTag*>*/
    int** sendShouts(QObject* obj, QString member, int c); /*QList<HShout*>*/

    void sendTracks_2(QStringList);  /*for internal use*/
    void sendTags_2(QStringList); /*for internal use*/
    void sendPic_2_0(QString pic) { sendPic_2((PictureSize)0,pic); }
    void sendPic_2_1(QString pic) { sendPic_2((PictureSize)1,pic); }
    void sendPic_2_2(QString pic) { sendPic_2((PictureSize)2,pic); }
    void sendPic_2_3(QString pic) { sendPic_2((PictureSize)3,pic); }
    void sendPic_2(HAlbum::PictureSize p,QString pic); /*for internal use*/
    void sendMoreTags_2(QStringList); /*for internal use*/
    void sendShouts_2(QStringList);
    void sendShouts_3(QStringList);
    void sendShouts_4(QStringList);

    void removeFromQueue(QObject*o) {
        for(int i=0;i<s_trackQueue.size();i++) {
            if(s_trackQueue[i].first==o) {
                s_trackQueue.removeAt(i);
                --i;
            }
        }
        for(int i=0;i<s_tagQueue.size();i++) {
            if(s_tagQueue[i].first==o) {
                s_tagQueue.removeAt(i);
                --i;
            }
        }
        for(int i=0;i<s_moreTagQueue.size();i++) {
            if(s_moreTagQueue[i].first==o) {
                s_moreTagQueue.removeAt(i);
                --i;
            }
        }
        for(int j=0;j<4;j++) {
            for(int i=0;i<s_picQueue[j].size();i++) {
                if(s_picQueue[j][i].first==o) {
                    s_picQueue[j].removeAt(i);
                    --i;
                }
            }
        }
    }

public:
    struct LIBHATHORSHARED_EXPORT ShoutData : public HCachedInfo {
        ShoutData(QString artist,QString album);
        bool process(const QString& data);
    } s_shoutData;


//    QImage getPic(PictureSize size);
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

    //Degenerate copy and assignment
    HAlbum(const HAlbum&);
    HAlbum& operator=(const HAlbum&);
};

#endif // HALBUM_H
