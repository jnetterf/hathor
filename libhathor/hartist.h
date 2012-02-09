#ifndef HARTIST_H
#define HARTIST_H

#include "hobject.h"
#include <QStringList>
#include <QPixmap>
#include <QVariant>
#include <QObject>
#include <QTimer>
#include "halbum.h"
#include "htrack.h"
#include "hshout.h"
#include "libhathor_global.h"

struct HTag;

struct LIBHATHORSHARED_EXPORT ArtistShoutData : public QObject {
    Q_OBJECT
    friend class HArtist;
private:
    QMutex mutex, mutex_2;
    QString artist;
    QList<HShout*> shouts;  QList< QPair<QObject*,QString> > shoutQueue;
    bool got;
    HRunOnceNotifier* getting;
    ArtistShoutData() : got(0), getting(0) {}
public slots:
    void sendData(QString artist);
    void sendData_process();
    void sendData_processQueue();
};

struct ExtraPictureData : QObject {
    Q_OBJECT
    friend class HArtist;
private:
    bool s_errored;
    QList<HCachedPixmap*> pics;
    QStringList pic_urls;
    bool got_urls;
    bool getting;
    QString s_artist;
    QSettings sett;

    struct HEPTriplet {
        QObject* first;
        QString second;
        int third;
        HEPTriplet(QObject* a,QString b,int c) : first(a), second(b), third(c) {}
    };
    QList<HEPTriplet> s_queue;

    void sendPics(QObject* o,QString m,int c);
    ExtraPictureData(QString artist);
public slots:
    void processPicUrls();
    void procQueue();
};

class LIBHATHORSHARED_EXPORT HArtist : public HObject
{
    Q_OBJECT
    QMutex queueMutex;
    HCachedPixmap* s_cachedPixmap[4];
    QList< QPair<QObject*,QString> > s_picQueue[4];
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_moreTagQueue;
    struct HTriple {
        QObject* first;
        QString second;
        int third;
        HTriple(QObject* a,const QString& b,const int& c) : first(a),second(b),third(c) {}
    };

    QList< HTriple > s_albumQueue;
    QList< HTriple > s_trackQueue;
    QList< HTriple > s_similarQueue;

public:
    static HArtist& get(QString name);
    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    bool isCached() { return s_infoData.isCached(); }

public slots:
    QString getName() { return s_name; }
    int** sendPic(PictureSize p,QObject* o,QString m); /* QPixmap */
    int** sendPicNames(PictureSize p,QObject* o,QString m,QObject* g=0); /* QString */
    int** sendTagNames(QObject* o,QString m,QObject* g=0); /* QStringList */
    int** sendTags(QObject* o,QString m); /* QList<HTag*> */
    int** sendMoreTagNames(QObject* o,QString m,QObject* g=0); /* QStringList */
    int** sendMoreTags(QObject* obj, const char* member); /*QList<HTag*>*/
    int** sendListenerCount(QObject* o,QString m); /*int*/
    int** sendPlayCount(QObject* o,QString m); /*int*/
    int** sendUserPlayCount(QObject* o,QString m); /*int*/
    int** sendBio(QObject* o,QString m); /*QString*/
    int** sendBioShort(QObject* o,QString m); /*QString*/
    int** sendAlbumsNames(QObject* o,QString m,QObject* g=0); /* QStringList */
    int** sendAlbums(QObject* o,QString m, int count=-1); /* multiple HAlbum* */
    int** sendTrackNames(QObject* o,QString m,QObject* g=0); /* QString */
    int** sendTracks(QObject* o,QString m, int count=-1); /* multiple HTrack* */
    int** sendSimilarNames(QObject* o,QString m,QObject* g=0); /* QStringList */
    int** sendSimilar(QObject* o,QString m, int count=-1); /* multiple HArtist* */
    int** sendShouts(QObject* o,QString m); /* QList<HShout*> */
    int** sendSimilarScores(QObject* o,QString m); /* QList<double> */
    void sendExtraPics(QObject* o,QString m, int count);    /* TO DO */

private:
    friend class HCachedInfo;
private slots:
    void sendPic_2_0(QString pic) { sendPic_2((PictureSize)0,pic); }
    void sendPic_2_1(QString pic) { sendPic_2((PictureSize)1,pic); }
    void sendPic_2_2(QString pic) { sendPic_2((PictureSize)2,pic); }
    void sendPic_2_3(QString pic) { sendPic_2((PictureSize)3,pic); }
    void sendMoreTags_2(QStringList); /*for internal use*/
private:
    static QHash<QString, HArtist*> _map;
    HArtist(QString name);  // use HArtist::get(name)

    QString s_name;

    struct LIBHATHORSHARED_EXPORT ArtistInfo : public HCachedInfo {
        ArtistInfo(QString artist);
        bool process(const QString& data);
    } s_infoData;

    struct LIBHATHORSHARED_EXPORT ArtistExtraTagData : public HCachedInfo {
        ArtistExtraTagData(QString artist);
        bool process(const QString& data);
    } s_extraTagData;

    struct LIBHATHORSHARED_EXPORT ArtistAlbumData : public HCachedInfo {
        ArtistAlbumData(QString artist);
        bool process(const QString& data);
    } s_albumData;

    struct LIBHATHORSHARED_EXPORT ArtistTrackData : public HCachedInfo {
        ArtistTrackData(QString artist);
        bool process(const QString& data);
    } s_trackData;

    struct LIBHATHORSHARED_EXPORT ArtistSimilarData : public HCachedInfo {
        ArtistSimilarData(QString artist);
        bool process(const QString& data);
    } s_similarData;


    ArtistShoutData s_shoutData;
    ExtraPictureData s_extraPictureData;

public slots:
    void sendPic_2(PictureSize p,QString pic); /* for internal use */
    void sendTags_2(QStringList); /* for internal use */
    void sendAlbums_2(QStringList); /* for internal use */
    void sendTracks_2(QStringList); /* for internal use */
    void sendSimilar_2(QStringList); /* for internal use */

private:
    //Degenerate copy and assignment
    HArtist(const HArtist&);
    HArtist& operator=(const HArtist&);
};

#endif // HARTIST_H
