#include "htrack.h"
#include "hartist.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QSettings>
#include <QVariant>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"
//#include "hrdiointerface.h"

QHash<QString, HTrack*> HTrack::_map;

HTrack::HTrack(QString artist, QString track) : s_artist(artist), s_track(track), s_albumAlbumNameCache_GOT(0), s_trackNameCache_GOT(0), s_infoData(artist,track), s_extraTagData(artist,track),
    s_similarData(artist,track), s_audioFeatureData(artist,track)
{
}

HTrack& HTrack::get(const QString &artist, const QString &track) {
    if(_map.value(artist+"__"+track,0)) { return *_map.value(artist+"__"+track);
    }

    _map.insert(artist+"__"+track,new HTrack(artist,track));
    return get(artist,track);
}

HArtist& HTrack::getArtist() const {
    return HArtist::get(getArtistName());
}

int** HTrack::sendPlayCount(QObject * o, QString m) { return s_infoData.sendProperty("playCount",o,m); }
int** HTrack::sendListenerCount(QObject * o, QString m) { return s_infoData.sendProperty("listeners",o,m); }
int** HTrack::sendUserPlayCount(QObject * o, QString m) { return s_infoData.sendProperty("userPlayCount",o,m); }
int** HTrack::sendTagNames(QObject * o, QString m,QObject* g) { return s_infoData.sendProperty("tagNames",o,m,g); }
int** HTrack::sendTags(QObject * o, QString m) {
    s_tagQueue.push_back(qMakePair(o,m));
    return sendTagNames(this,"sendTags_2",o);
}

void HTrack::sendTags_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    QList<HTag*> ret;
    for(int i=0;i<t.size();i++) {
        ret.push_back(&HTag::get(t[i]));
    }
    while(s_tagQueue.size()) {
        QPair< QObject*, QString > p=s_tagQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HTag*>,ret));
    }
    s_tagQueue.clear();
}

int** HTrack::sendMoreTagNames(QObject * o, QString m,QObject* g) { return s_extraTagData.sendProperty("tagNames",o,m,g); }

int** HTrack::sendMoreTags(QObject * o, QString m) {
    s_extraTagQueue.push_back(qMakePair(o,m));
    return sendMoreTagNames(this,"sendMoreTags_2",o);
}


void HTrack::sendMoreTags_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    QList<HTag*> ret;
    for(int i=0;i<t.size();i++) {
        ret.push_back(&HTag::get(t[i]));
    }
    while(s_extraTagQueue.size()) {
        QPair< QObject*, QString > p=s_extraTagQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HTag*>,ret));
    }
    s_extraTagQueue.clear();
}

int** HTrack::sendShouts(QObject * o, QString m) {
//    s_shoutData.shoutQueue.push_back(qMakePair(o,QString(m)));
//    s_shoutData.sendData(s_artist,s_track);
    return 0;
}

int** HTrack::sendSummary(QObject * o, QString m) { return s_infoData.sendProperty("summary",o,m); }
int** HTrack::sendContent(QObject * o, QString m) { return s_infoData.sendProperty("content",o,m); }
int** HTrack::sendLoved(QObject * o, QString m) { return s_infoData.sendProperty("loved",o,m); }
int** HTrack::sendAlbumNames(QObject * o, QString m,QObject* g) { return s_infoData.sendProperty("albumAlbumNames",o,m,g); }
int** HTrack::sendAlbumArtistNames(QObject * o, QString m,QObject* g) { return s_infoData.sendProperty("albumArtistNames",o,m,g); }

int** HTrack::sendAlbums(QObject * o, QString m) {
    s_albumQueue.push_back(qMakePair(o,m));
    return &(*sendAlbumNames(this,"sendAlbums_2",o)=*sendAlbumArtistNames(this,"sendAlbums_3",o));  // I <3 this line
}

void HTrack::sendAlbums_2(QStringList t) {
    s_albumAlbumNameCache=t;
    s_albumAlbumNameCache_GOT=1;
    if(s_albumAlbumNameCache.size()==s_albumArtistNameCache.size()) sendAlbums_3(s_albumArtistNameCache);
}

void HTrack::sendAlbums_3(QStringList t) {
    s_albumArtistNameCache=t;
    if(s_albumAlbumNameCache_GOT) {
        QMutexLocker locker(&queueMutex);
        QList<HAlbum*> ret;
        for(int i=0;i<s_albumAlbumNameCache.size();i++) {
            ret.push_back(&HAlbum::get(s_albumArtistNameCache[i],s_albumAlbumNameCache[i]));
        }
        while(s_albumQueue.size()) {
            QPair< QObject*, QString > p=s_albumQueue.takeFirst();
            QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HAlbum*>,ret));
        }
        s_albumQueue.clear();
    }
}

int** HTrack::sendSimilarTrackNames(QObject *o, QString m,QObject* g) { return s_similarData.sendProperty("similarNames",o,m,g); }
int** HTrack::sendSimilarTrackArtistNames(QObject *o, QString m, QObject* g) { return s_similarData.sendProperty("similarArtistNames",o,m,g); }

int** HTrack::sendSimilar(QObject * o, QString m, int s) {
    s_similarQueue.push_back(HTrackTriplet(o,m,s));
    return &(*sendSimilarTrackNames(this,"sendSimilar_2")=*sendSimilarTrackArtistNames(this,"sendSimilar_3"));
}

void HTrack::sendSimilar_2(QStringList t) {
    QMutexLocker locker(&queueMutex);
    s_trackNameCache=t;
    s_trackNameCache_GOT=1;
    if(s_trackArtistNameCache.size()==s_trackNameCache.size()) {
        locker.unlock();
        sendSimilar_3(s_trackArtistNameCache);
    }
}

void HTrack::sendSimilar_3(QStringList t) {
    s_trackArtistNameCache=t;
    if(s_trackNameCache_GOT) {
        QMutexLocker locker(&queueMutex);
        QList<HTrack*> ret;
        for(int i=0;i<s_trackNameCache.size();i++) {
            ret.push_back(&HTrack::get(s_trackArtistNameCache[i],s_trackNameCache[i]));
        }
        while(s_similarQueue.size()) {
            HTrackTriplet p=s_similarQueue.takeFirst();
            for(int i=0;i<ret.size()&&(p.third==-1||i<p.third);i++) {
                QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HTrack*,ret[i]));
            }
            if(p.third>=ret.size()) QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HTrack*,0));
        }
        s_similarQueue.clear();
    }
}

int** HTrack::sendSimilarScores(QObject * o, QString m) { return s_similarData.sendProperty("similarScores",o,m); }

int** HTrack::sendBpm(QObject*o,QString m) { return s_audioFeatureData.sendProperty("bpm",o,m); }
int** HTrack::sendValence(QObject*o,QString m) { return s_audioFeatureData.sendProperty("valence",o,m); }
int** HTrack::sendAggression(QObject*o,QString m) { return s_audioFeatureData.sendProperty("aggression",o,m); }
int** HTrack::sendAvgLoudness(QObject*o,QString m) { return s_audioFeatureData.sendProperty("avg_loudness",o,m); }
int** HTrack::sendPeakLoudness(QObject*o,QString m) { return s_audioFeatureData.sendProperty("peak_loudness",o,m); }
int** HTrack::sendPercussiveness(QObject*o,QString m) { return s_audioFeatureData.sendProperty("percussiveness",o,m); }
int** HTrack::sendNoininess(QObject*o,QString m) { return s_audioFeatureData.sendProperty("noisiness",o,m); }
int** HTrack::sendGearshift(QObject*o,QString m) { return s_audioFeatureData.sendProperty("gearshift",o,m); }
int** HTrack::sendKey(QObject*o,QString m) { return s_audioFeatureData.sendProperty("key",o,m); } /* int */
int** HTrack::sendHarmonicCreativity(QObject*o,QString m) { return s_audioFeatureData.sendProperty("harmonic_creativity",o,m); }
int** HTrack::sendSmoothness(QObject*o,QString m) { return s_audioFeatureData.sendProperty("smoothness",o,m); }
int** HTrack::sendDanceability(QObject*o,QString m) { return s_audioFeatureData.sendProperty("danceability",o,m); }
int** HTrack::sendEnergy(QObject*o,QString m) { return s_audioFeatureData.sendProperty("energy",o,m); }
int** HTrack::sendSoundCreativity(QObject*o,QString m) { return s_audioFeatureData.sendProperty("sound_creativity",o,m); }
int** HTrack::sendPunch(QObject*o,QString m) { return s_audioFeatureData.sendProperty("punch",o,m); }
int** HTrack::sendTuning(QObject*o,QString m) { return s_audioFeatureData.sendProperty("tuning",o,m); }
int** HTrack::sendChordalClarity(QObject*o,QString m) { return s_audioFeatureData.sendProperty("chordal_clarity",o,m); }
int** HTrack::sendTempoInstability(QObject*o,QString m) { return s_audioFeatureData.sendProperty("tempo_instability",o,m); }
int** HTrack::sendRhythmicIntricacy(QObject*o,QString m) { return s_audioFeatureData.sendProperty("rhythmic_intricacy",o,m); }
int** HTrack::sendSpeed(QObject*o,QString m) { return s_audioFeatureData.sendProperty("speed",o,m); }

//////////////////////////////////////////////////////////////////////////////////////////////

HTrack::InfoData::InfoData(QString artist,QString track) {
    QMap<QString, QString> params;
    params["method"] = "track.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["track"] = track;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+track.toUtf8()+"TRACKINFO",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("tagNames",b);
    addProperty<QString>("summary",b);
    addProperty<QString>("content",b);
    addProperty<QString>("listeners",b);

    addProperty<int>("playCount",b);
    addProperty<int>("userPlayCount",b);
    addProperty<int>("listeners",b);
    addProperty<bool>("loved",b);
    addProperty<QStringList>("albumArtistNames",b);
    addProperty<QStringList>("albumAlbumNames",b);
}

bool HTrack::InfoData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );

        QDomElement element = doc.documentElement();

        QStringList albumArtists;
        QStringList albumAlbums;
        QStringList tags;

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "listeners") setProperty("listeners",l.toText().data().toInt());
                    else if ( m.nodeName() == "playcount") setProperty("playCount",l.toText().data().toInt());
                    else if ( m.nodeName() == "userplaycount" ) setProperty("userPlayCount",l.toText().data().toInt());
                    else if ( m.nodeName() == "userloved" ) setProperty("loved",(bool)l.toText().data().toInt());

                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) setProperty("summary", HObject::eliminateHtml(k.toText().data()));
                        else if ( l.nodeName() == "content" ) {
                            QString content = k.toText().data();
                            setProperty("content",HObject::eliminateHtml(content));
                        }
                        else if ( l.nodeName() == "artist" ) albumArtists.push_back(k.toText().data());
                        else if ( l.nodeName() == "title" ) albumAlbums.push_back(k.toText().data());
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) tags.append( j.toText().data() );
                        }
                    }
                }
            }
        }

        setProperty("albumArtistNames",albumArtists);
        setProperty("albumAlbumNames",albumAlbums);
        setProperty("tagNames",tags);

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HTrack::ExtraTagData::ExtraTagData(QString artist, QString track) {
    QMap<QString, QString> params;
    params["method"] = "track.getTopTags";
    params["artist"] = artist;
    params["track"] = track;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+track.toUtf8()+"TRACKTAGS",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("tagNames",b);
}

bool HTrack::ExtraTagData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );
        QStringList tags;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) tags.push_back( k.toText().data() );
                    }
                }
            }
        }

        setProperty("tagNames",tags);

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HTrack::SimilarData::SimilarData(QString artist,QString track) {
    QMap<QString, QString> params;
    params["method"] = "track.getSimilar";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["track"] = track;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+track.toUtf8()+"TRACKSIMILAR",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("similarNames",b);
    addProperty<QStringList>("similarArtistNames",b);
    addProperty<QList<double> >("similarScores",b);
}

bool HTrack::SimilarData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );
        QStringList similar;
        QList<double> score;
        QStringList artists;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "name" ) similar.push_back( k.toText().data() );
                        if ( l.nodeName() == "match" ) score.push_back( k.toText().data().toDouble() );
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( k.nodeName() == "name" ) artists.append( j.toText().data() );
                        }
                    }
                }
            }
        }
        setProperty("similarNames",similar);
        setProperty("similarArtistNames",artists);
        setProperty("similarScores",score);
    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

HTrack::AudioFeatures::AudioFeatures(QString artist,QString track) {
    QMap<QString, QString> params;
    params["method"] = "track.getAudioFeatures";
    params["artist"] = artist;
    params["track"] = track;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+track.toUtf8()+"TRACKAF",QCryptographicHash::Md5).toHex();
    addProperty<int>("bpm",b);
    addProperty<double>("valence",b);
    addProperty<double>("aggression",b);
    addProperty<double>("avg_loudness",b);
    addProperty<double>("peak_loudness",b);
    addProperty<double>("rhythmic_regularity",b);
    addProperty<double>("percussiveness",b);
    addProperty<double>("noisiness",b);
    addProperty<double>("gearshift",b);
    addProperty<int>("key",b);
    addProperty<double>("harmonic_creativity",b);
    addProperty<double>("smoothness",b);
    addProperty<double>("danceability",b);
    addProperty<double>("energy",b);
    addProperty<double>("punch",b);
    addProperty<double>("sound_creativity",b);
    addProperty<double>("tuning",b);
    addProperty<double>("chordal_clarity",b);
    addProperty<double>("tempo_instability",b);
    addProperty<double>("rhythmic_intricacy",b);
    addProperty<double>("speed",b);

}

bool HTrack::AudioFeatures::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if(m.nodeName()=="bpm") setProperty("bpm",l.toText().data().toInt());
                    if(m.nodeName()=="valence") setProperty("valence",l.toText().data().toDouble());
                    if(m.nodeName()=="aggression") setProperty("aggression",l.toText().data().toDouble());
                    if(m.nodeName()=="avg_loudness") setProperty("avg_loudness",l.toText().data().toDouble());
                    if(m.nodeName()=="peak_loudness") setProperty("peak_loudness",l.toText().data().toDouble());
                    if(m.nodeName()=="rhythmic_regularity") setProperty("rhythmic_regularity",l.toText().data().toDouble());
                    if(m.nodeName()=="percussiveness") setProperty("percussiveness",l.toText().data().toDouble());
                    if(m.nodeName()=="noisiness") setProperty("noisiness",l.toText().data().toDouble());
                    if(m.nodeName()=="gearshift") setProperty("gearshift",l.toText().data().toDouble());
                    if(m.nodeName()=="key") setProperty("key",l.toText().data().toInt());
                    if(m.nodeName()=="harmonic_creativity") setProperty("harmonic_creativity",l.toText().data().toDouble());
                    if(m.nodeName()=="smoothness") setProperty("smoothness",l.toText().data().toDouble());
                    if(m.nodeName()=="danceability") setProperty("danceability",l.toText().data().toDouble());
                    if(m.nodeName()=="energy") setProperty("energy",l.toText().data().toDouble());
                    if(m.nodeName()=="punch") setProperty("punch",l.toText().data().toDouble());
                    if(m.nodeName()=="sound_creativity") setProperty("sound_creativity",l.toText().data().toDouble());
                    if(m.nodeName()=="tuning") setProperty("tuning",l.toText().data().toDouble());
                    if(m.nodeName()=="chordal_clarity") setProperty("chordal_clarity",l.toText().data().toDouble());
                    if(m.nodeName()=="tempo_instability") setProperty("tempo_instability",l.toText().data().toDouble());
                    if(m.nodeName()=="rhythmic_intricacy") setProperty("rhythmic_intricacy",l.toText().data().toDouble());
                    if(m.nodeName()=="speed") setProperty("speed",l.toText().data().toDouble());
                }
            }
        }
//        setProperty("similarNames",similar);
//        setProperty("similarArtistNames",artists);
//        setProperty("similarScores",score);
    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}


void TrackShoutData::sendData(QString artist,QString track) {
    QMutexLocker lock(&mutex);  // DO NOT USE QMetaObject::invokeMethod
    this->artist=artist;
    this->track=track;

    if(getting) connect(getting,SIGNAL(notify()),this,SLOT(sendData_processQueue()));
    else if(got) sendData_processQueue();
    else {
        // no cache?
        QMap<QString, QString> params;
        params["method"] = "track.getShouts";
        params["artist"] = artist;
        params["track"] = track;
        QNetworkReply* reply = lastfmext_post( params );

        getting=new HRunOnceNotifier;   // !!
        connect(reply,SIGNAL(finished()),this,SLOT(sendData_process()));
    }
}

void TrackShoutData::sendData_process() {
    QNetworkReply* reply=qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        qDebug()<<"GOT ERROR - INVALID DATA RECORDED!";
        getting->emitNotify();
        getting=0;
        return;
    }

    try {
        QString body,author,date;
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "body" ) body = k.toText().data();
                        else if ( l.nodeName() == "author" ) author = k.toText().data();
                        else if ( l.nodeName() == "date") {
                            date = k.toText().data();
                            shouts.push_back(new HShout(body,HUser::get(author),date));
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }

    got=1;    // !!
    getting->emitNotify();
    getting=0;
    sendData_processQueue();
}

void TrackShoutData::sendData_processQueue() {
    QMutexLocker locker(&mutex_2);
    while(shoutQueue.size()) {
        QPair< QObject*, QString > p=shoutQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HShout*>,shouts));
    }
    shoutQueue.clear();
}
