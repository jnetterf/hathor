#include "htrack.h"
#include "hartist.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QEventLoop>
#include <QSettings>
#include <QVariant>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"

QMap<QString, HTrack*> HTrack::_map;

HTrack::HTrack(QString artist, QString track) : s_artist(artist), s_track(track)
{
}

HTrack& HTrack::get(QString artist, QString track) {
    if(_map.value(artist+"__"+track,0)) {
        return *_map.value(artist+"__"+track);
    }

    _map.insert(artist+"__"+track,new HTrack(artist,track));
    return get(artist,track);
}

int HTrack::getPlayCount() {
    if(s_infoData.got) {
        return s_infoData.playCount;
    }
    s_infoData.getData(s_artist,s_track);
    return getPlayCount();
}

int HTrack::getListenerCount() {
    if(s_infoData.got) {
        return s_infoData.listenerCount;
    }
    s_infoData.getData(s_artist,s_track);
    return getListenerCount();
}

int HTrack::getUserPlayCount() {
    if(s_infoData.got) {
        return s_infoData.userPlayCount;
    }
    s_infoData.getData(s_artist,s_track);
    return getUserPlayCount();
}

HArtist& HTrack::getArtist() {
    return HArtist::get(getArtistName());
}

QStringList HTrack::getTagNames() {
    if(s_infoData.got) {
        return s_infoData.tags;
    }
    s_infoData.getData(s_artist,s_track);
    return getTagNames();
}

QList<HTag*> HTrack::getTags() {
    QStringList tags=getTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}

QList<HTag*> HTrack::getMoreTags() {
    QStringList tags=getMoreTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}

QStringList HTrack::getMoreTagNames() {
    if(s_extraTagData.got) {
        return s_extraTagData.tags;
    }
    s_extraTagData.getData(s_artist,s_track);
    return getMoreTagNames();
}

QList<HShout*> HTrack::getShouts() {
    if(s_shoutData.got) return s_shoutData.shouts;
    s_shoutData.getData(s_artist,s_track);
    return getShouts();
}

QString HTrack::getSummary() {
    if(s_infoData.got) {
        return s_infoData.summary;
    }
    s_infoData.getData(s_artist,s_track);
    return getSummary();
}

QString HTrack::getContent() {
    if(s_infoData.got) {
        return s_infoData.content;
    }
    s_infoData.getData(s_artist,s_track);
    return getContent();
}

bool HTrack::getLoved() {
    if(s_infoData.got) {
        return s_infoData.loved;
    }
    s_infoData.getData(s_artist,s_track);
    return getLoved();
}

QStringList HTrack::getAlbumNames() {
    if(s_infoData.got) {
        return s_infoData.albumAlbums;
    }
    s_infoData.getData(s_artist,s_track);
    return getAlbumNames();
}


QStringList HTrack::getAlbumArtistNames() {
    if(s_infoData.got) {
        return s_infoData.albumArtists;
    }
    s_infoData.getData(s_artist,s_track);
    return getAlbumArtistNames();
}


QList<HAlbum*> HTrack::getAlbums() {
    QStringList a=getAlbumNames();
    QStringList b=getAlbumArtistNames();
    QList<HAlbum*> ret;
    for(int i=0;i<a.size();i++) {
        ret.push_back(&HAlbum::get(b[i],a[i]));
    }
    return ret;
}

QList<HTrack*> HTrack::getSimilar() {
    if(s_similarData.got) {
        QList<HTrack*> ret;
        for(int i=0;i<s_similarData.similar.size();i++) {
            ret.push_back(&HTrack::get(s_similarData.artists[i],s_similarData.similar[i]));
        }
        return ret;
    }
    s_similarData.getData(s_artist,s_track);
    return getSimilar();
}

QList<double> HTrack::getSimilarScores() {

    if(s_similarData.got) {
        QList<double> ret;
        for(int i=0;i<s_similarData.similar.size();i++) {
            ret.push_back(s_similarData.score[i].toDouble());
        }
        return ret;
    }
    s_similarData.getData(s_artist,s_track);
    return getSimilarScores();
}


//////////////////////////////////////////////////////////////////////////////////////////////

void HTrack::InfoData::getData(QString artist,QString track) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","trackInfo");
    QString dumbName=artist+"__"+track;
    if(sett.value("cache for "+dumbName,0).toInt()==4) {
        tags=sett.value("tags for "+dumbName).toStringList();
        summary=sett.value("summary for "+dumbName).toString();
        content=sett.value("content for "+dumbName).toString();
        listenerCount=sett.value("listeners for "+dumbName).toInt();
        playCount=sett.value("playcount for "+dumbName).toInt();
        userPlayCount=sett.value("userplaycount for "+dumbName).toInt();
        loved=sett.value("loved for "+dumbName).toBool();
        albumArtists=sett.value("albumArtists for "+dumbName).toStringList();
        albumAlbums=sett.value("albumAlbums for "+dumbName).toStringList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "track.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["track"] = track;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        getData(artist,track);
        return;
    }

    listenerCount=0;
    playCount=0;
    userPlayCount=0;
    summary="";
    content="";
    loved=0;
    try {
        QDomDocument doc;
        doc.setContent( reply->readAll() );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "listeners") listenerCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "playcount") playCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "userplaycount" ) userPlayCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "userloved" ) loved = l.toText().data().toInt();

                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) summary = k.toText().data();
                        else if ( l.nodeName() == "content" ) content = k.toText().data();
                        else if ( l.nodeName() == "artist" ) albumArtists.push_back(k.toText().data());
                        else if ( l.nodeName() == "title" ) albumAlbums.push_back(k.toText().data());
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) tags.append( j.toText().data() );
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    content.replace((char)13,"<br>");
    sett.setValue("cache for "+dumbName,4);
    sett.setValue("tags for "+dumbName,tags);
    sett.setValue("listeners for "+dumbName,listenerCount);
    sett.setValue("playcount for "+dumbName,playCount);
    sett.setValue("userplaycount for "+dumbName,userPlayCount);
    sett.setValue("summary for "+dumbName,summary);
    sett.setValue("content for "+dumbName,content);
    sett.setValue("loved for "+dumbName,loved);
    sett.setValue("albumArtists for "+dumbName,albumArtists);
    sett.setValue("albumAlbums for "+dumbName,albumAlbums);
}


void HTrack::ExtraTagData::getData(QString artist,QString track) {
    if(got) {
        return;
    }
    got=1;

    QString cacheName=artist+"__"+track;

    QSettings sett("hathorMP","extraTagData");
    if(sett.value("cache for "+cacheName,0).toInt()==2) {
        tags=sett.value("tags for "+cacheName).toStringList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "tracks.getTopTags";
    params["artist"] = artist;
    params["track"] = track;
    QNetworkReply* reply = lastfmext_post( params );

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        getData(artist,track);
        return;
    }

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    try {
        QDomDocument doc;
        doc.setContent( reply->readAll() );

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

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+cacheName,2);
    sett.setValue("tags for "+cacheName,tags);
}


void HTrack::ShoutData::getData(QString artist,QString track) {
    if(got) {
        return;
    }
    got=1;

    // no cache?

    QMap<QString, QString> params;
    params["method"] = "track.getShouts";
    params["artist"] = artist;
    params["track"] = track;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        getData(artist,track);
        return;
    }

    try {
        QString body,author,date;
        QDomDocument doc;
        doc.setContent( reply->readAll() );

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
}

void HTrack::SimilarData::getData(QString artist,QString track) {
    if(got) {
        return;
    }
    got=1;

    QString dumbname=artist+"__"+track;

    QSettings sett("hathorMP","trackSimilar");
    if(sett.value("cache for "+dumbname,0).toInt()==3) {
        similar=sett.value("similar for "+dumbname).toStringList();
        artists=sett.value("artists for "+dumbname).toStringList();
        score=sett.value("score for "+dumbname).toList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "track.getSimilar";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["track"] = track;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        getData(artist,track);
        return;
    }

    try {
        QDomDocument doc;
        doc.setContent( reply->readAll() );

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

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+dumbname,3);
    sett.setValue("similar for "+dumbname,similar);
    sett.setValue("score for "+dumbname,score);
    sett.setValue("artists for "+dumbname,artists);
}
