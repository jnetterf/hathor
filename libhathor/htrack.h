#ifndef HTRACK_H
#define HTRACK_H

#include "hobject.h"
#include "hnotifier.h"
#include "hfuture.h"
#include "libhathor_global.h"
#include <QStringList>
#include <QPixmap>
#include <QVariant>

class HArtist;
class HAlbum;
class HTag;
class HShout;

struct LIBHATHORSHARED_EXPORT TrackShoutData : public QObject {
    Q_OBJECT
    friend class HTrack;
private:
    QMutex mutex, mutex_2;
    QString artist, track;
    QList<HShout*> shouts;  QList< QPair<QObject*,QString> > shoutQueue;
    bool got;
    HRunOnceNotifier* getting;
    TrackShoutData() : got(0), getting(0) {}
public slots:
    void sendData(QString artist, QString track);
    void sendData_process();
    void sendData_processQueue();
};

class LIBHATHORSHARED_EXPORT HTrack : public HObject
{
    Q_OBJECT
    QString s_artist;
    QString s_track;

    QMutex queueMutex;
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_extraTagQueue;
    QList< QPair<QObject*,QString> > s_albumQueue;
    QList< QPair<QObject*,QString> > s_similarQueue;

    QStringList s_albumAlbumNameCache;
    QStringList s_albumArtistNameCache;
    bool s_albumAlbumNameCache_GOT;


    QStringList s_trackNameCache;
    QStringList s_trackArtistNameCache;
    bool s_trackNameCache_GOT;

public:
    static HTrack& get(QString artist, QString trackName);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    QString getArtistName() { return s_artist; }
    QString getTrackName() { return s_track; }
    HArtist& getArtist();

    bool isCached() { return s_infoData.isCached(); }

public slots:
    void sendTagNames(QObject*,QString); /* QStringList */
    void sendMoreTagNames(QObject*,QString); /* QStringList */
    void sendTags(QObject*,QString); /* QList<HTag*> */
    void sendTags_2(QStringList); /* for internal use */
    void sendMoreTags(QObject*,QString); /* QList<HTag*> */
    void sendMoreTags_2(QStringList); /* for internal use */

    void sendShouts(QObject*,QString); /* QList<HShout*> */

    void sendListenerCount(QObject*,QString); /* int */
    void sendPlayCount(QObject*,QString); /* int */
    void sendUserPlayCount(QObject*,QString); /* int */
    void sendSummary(QObject*,QString); /* QString */
    void sendContent(QObject*,QString); /* QString */
    void sendLoved(QObject*,QString); /* bool */

    void sendAlbumNames(QObject*,QString); /* QStringList */
    void sendAlbumArtistNames(QObject*,QString); /* QStringList */
    void sendAlbums(QObject*,QString); /* QList<HAlbum*> */
    void sendAlbums_2(QStringList); /* for internal use */
    void sendAlbums_3(QStringList); /* for internal use */

    void sendSimilarTrackNames(QObject*,QString); /* QStringList */
    void sendSimilarTrackArtistNames(QObject*,QString); /* QStringList */
    void sendSimilar(QObject*,QString); /* QList<HTrack*> */
    void sendSimilar_2(QStringList); /* for internal use */
    void sendSimilar_3(QStringList); /* for internal use */
    void sendSimilarScores(QObject*,QString); /* QList<double> */


    /* the following are doubles, except where noted: */
    void sendBpm(QObject*,QString);
    void sendValence(QObject*,QString);
    void sendAggression(QObject*,QString);
    void sendAvgLoudness(QObject*,QString);
    void sendPeakLoudness(QObject*,QString);
    void sendPercussiveness(QObject*,QString);
    void sendNoininess(QObject*,QString);
    void sendGearshift(QObject*,QString);
    void sendKey(QObject*,QString); /* int */
    void sendHarmonicCreativity(QObject*,QString);
    void sendSmoothness(QObject*,QString);
    void sendDanceability(QObject*,QString);
    void sendEnergy(QObject*,QString);
    void sendPunch(QObject*,QString);
    void sendSoundCreativity(QObject*,QString);
    void sendTuning(QObject*,QString);
    void sendChordalClarity(QObject*,QString);
    void sendTempoInstability(QObject*,QString);
    void sendRhythmicIntricacy(QObject*,QString);
    void sendSpeed(QObject*,QString);

private:
    static QHash<QString, HTrack*> _map;
    HTrack(QString artist, QString track);  // use HTrack::get(name)

    struct InfoData : HCachedInfo {
        InfoData(QString artist, QString track);
        bool process(const QString& data);
    } s_infoData;

    struct ExtraTagData : HCachedInfo {
        ExtraTagData(QString artist, QString track);
        bool process(const QString &data);
    } s_extraTagData;

    struct SimilarData : HCachedInfo {
        SimilarData(QString artist, QString track);
        bool process(const QString &data);
    } s_similarData;

    struct AudioFeatures : HCachedInfo {
        AudioFeatures(QString artist, QString track);
        bool process(const QString &data);
    } s_audioFeatureData;

    TrackShoutData s_shoutData;

private:
    //Degenerate copy and assignment
    HTrack(const HTrack&);
    HTrack& operator=(const HTrack&);
};

#endif // HTRACK_H
