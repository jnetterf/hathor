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
public:
    /*!
     * Use this function to gain access to a track and its information.
     */
    static HTrack& get(const QString& artist, const QString& trackName);

    const QString& getArtistName() const { return s_artist; }
    const QString& getTrackName() const { return s_track; }
    HArtist& getArtist() const;

public slots:
    /*!
     * Asynchronously sends a QStringList of 3-4 tag names (for example: "rock", "pop",...).
     *
     * This function follows the send pattern found throughout Hathor. See Hathor's README for more information.
     * This function will activate a signal or slot containing accepting exactly one parameter: a QStringList
     * This parameter will contain a small list of tags.
     *
     * \param[in] object the object this property should be sent to.
     * \param[in] slot the slot (or signal) WITHOUT DECORATION - i.e., "aSlot" not SLOT(aSlot(...)) - within object this property should be sent to.
     * \param[in] guest the object which owns the priority: set this if you are requesting this property on behalf on another object.
     * \return a double-pointer to an integer where you can set the priority of this action. 0 <= priority <= 100. you can change where this points to.
     * \return sometimes, when two properties are loaded together, the same double-pointer can affect more than one object. therefore, you should assign
     * \return low-priorities before high-priorities.
     * \sa sendMoreTagNames sendTags sendMoreTags
     */
    int** sendTagNames(QObject* object,QString slot,QObject* guest=0); /* QStringList */
    int** sendMoreTagNames(QObject*,QString,QObject* guest=0); /* QStringList */
    int** sendTags(QObject*,QString); /* QList<HTag*> */
    int** sendMoreTags(QObject*,QString); /* QList<HTag*> */

    int** sendShouts(QObject*,QString); /* QList<HShout*> */

    int** sendListenerCount(QObject*,QString); /* int */
    int** sendPlayCount(QObject*,QString); /* int */
    int** sendUserPlayCount(QObject*,QString); /* int */
    int** sendSummary(QObject*,QString); /* QString */
    int** sendContent(QObject*,QString); /* QString */
    int** sendLoved(QObject*,QString); /* bool */

    int** sendAlbumNames(QObject*,QString,QObject* guest=0); /* QStringList */
    int** sendAlbumArtistNames(QObject*,QString,QObject* guest=0); /* QStringList */
    int** sendAlbums(QObject*,QString); /* QList<HAlbum*> */

    int** sendSimilarTrackNames(QObject*,QString,QObject* guest=0); /* QStringList */
    int** sendSimilarTrackArtistNames(QObject*,QString,QObject* guest=0); /* QStringList */
    int** sendSimilar(QObject*,QString,int=-1); /* QList<HTrack*> */
    int** sendSimilarScores(QObject*,QString); /* QList<double> */


    /* the following are doubles, except where noted: */
    int** sendBpm(QObject*,QString);
    int** sendValence(QObject*,QString);
    int** sendAggression(QObject*,QString);
    int** sendAvgLoudness(QObject*,QString);
    int** sendPeakLoudness(QObject*,QString);
    int** sendPercussiveness(QObject*,QString);
    int** sendNoininess(QObject*,QString);
    int** sendGearshift(QObject*,QString);
    int** sendKey(QObject*,QString); /* int */
    int** sendHarmonicCreativity(QObject*,QString);
    int** sendSmoothness(QObject*,QString);
    int** sendDanceability(QObject*,QString);
    int** sendEnergy(QObject*,QString);
    int** sendPunch(QObject*,QString);
    int** sendSoundCreativity(QObject*,QString);
    int** sendTuning(QObject*,QString);
    int** sendChordalClarity(QObject*,QString);
    int** sendTempoInstability(QObject*,QString);
    int** sendRhythmicIntricacy(QObject*,QString);
    int** sendSpeed(QObject*,QString);

private slots:
    void sendTags_2(QStringList);
    void sendMoreTags_2(QStringList);
    void sendAlbums_2(QStringList);
    void sendAlbums_3(QStringList);
    void sendSimilar_2(QStringList);
    void sendSimilar_3(QStringList);
private:
    friend class HCachedInfo;

    struct HTrackTriplet {
        QObject* first;
        QString second;
        int third;
        HTrackTriplet(QObject* co,QString cm,int cc) : first(co), second(cm), third(cc) {}
    };

    QString s_artist;
    QString s_track;

    QMutex queueMutex;
    QList< QPair<QObject*,QString> > s_tagQueue;
    QList< QPair<QObject*,QString> > s_extraTagQueue;
    QList< QPair<QObject*,QString> > s_albumQueue;
    QList< HTrackTriplet > s_similarQueue;

    QStringList s_albumAlbumNameCache;
    QStringList s_albumArtistNameCache;
    bool s_albumAlbumNameCache_GOT;


    QStringList s_trackNameCache;
    QStringList s_trackArtistNameCache;
    bool s_trackNameCache_GOT;

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
public:
    /*!
      For debugging.
     */
    bool isCached() { return s_infoData.isCached(); }

private:
    //Degenerate copy and assignment
    HTrack(const HTrack&);
    HTrack& operator=(const HTrack&);
};

#endif // HTRACK_H
