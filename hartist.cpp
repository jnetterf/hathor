#include "hartist.h"
#include "htrack.h"
#include "htag.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QEventLoop>
#include <QSettings>
#include <lastfm/ws.h>
#include <QDomDocument>
#include "lastfmext.h"

QMap<QString, HArtist*> HArtist::_map;

HArtist& HArtist::get(QString name) {
    if(_map.value(name,0)) {
        return *_map.value(name);
    }

    _map.insert(name,new HArtist(name));
    return get(name);
}

QPixmap HArtist::getPic(PictureSize p) {
    if(s_pictureData.got[p]) return s_pictureData.pics[p];
    if(s_infoData.got) {
        while(s_infoData.pics[p].isEmpty()&&p) p=(PictureSize)((int)p-1);
        s_pictureData.getData(s_infoData.pics[p],p);
        return getPic(p);
    }
    s_infoData.getData(s_name);
    return getPic(p);
}

QStringList HArtist::getTagNames() {
    if(s_infoData.got) {
        return s_infoData.tags;
    }
    s_infoData.getData(s_name);
    return getTagNames();
}


QList<HTag*> HArtist::getTags() {
    QStringList tags=getTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}

QStringList HArtist::getMoreTagNames() {
    if(s_extraTagData.got) {
        return s_extraTagData.tags;
    }
    s_extraTagData.getData(s_name);
    return getMoreTagNames();
}


QList<HTag*> HArtist::getMoreTags() {
    QStringList tags=getMoreTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}


int HArtist::getListenerCount() {
    if(s_infoData.got) {
        return s_infoData.listenerCount;
    }
    s_infoData.getData(s_name);
    return getListenerCount();
}

int HArtist::getPlayCount() {
    if(s_infoData.got) {
        return s_infoData.playCount;
    }
    s_infoData.getData(s_name);
    return getPlayCount();
}

int HArtist::getUserPlayCount() {
    if(s_infoData.got) {
        return s_infoData.userPlayCount;
    }
    s_infoData.getData(s_name);
    return getUserPlayCount();
}

QString HArtist::getBio() {
    if(s_infoData.got) {
        return s_infoData.bio;
    }
    s_infoData.getData(s_name);
    return getBio();
}

QString HArtist::getBioShort() {
    if(s_infoData.got) {
        return s_infoData.bioShort;
    }
    s_infoData.getData(s_name);
    return getBioShort();
}

QList<HAlbum *> HArtist::getAlbums() {
    if(s_albumData.got) {
        QList<HAlbum*> ret;
        for(int i=0;i<s_albumData.albums.size();i++) {
            ret.push_back(&HAlbum::get(s_name,s_albumData.albums[i]));
        }
        return ret;
    }
    s_albumData.getData(s_name);
    return getAlbums();
}

QList<HTrack *> HArtist::getTracks() {
    if(s_trackData.got) {
        QList<HTrack*> ret;
        for(int i=0;i<s_trackData.tracks.size();i++) {
            ret.push_back(&HTrack::get(s_name,s_trackData.tracks[i]));
        }
        return ret;
    }
    s_trackData.getData(s_name);
    return getTracks();
}

QList<HArtist*> HArtist::getSimilar() {
    if(s_similarData.got) {
        QList<HArtist*> ret;
        for(int i=0;i<s_similarData.similar.size();i++) {
            ret.push_back(&HArtist::get(s_similarData.similar[i]));
        }
        return ret;
    }
    s_similarData.getData(s_name);
    return getSimilar();
}

QList<HShout*> HArtist::getShouts() {
    if(s_shoutData.got) return s_shoutData.shouts;
    s_shoutData.getData(s_name);
    return getShouts();
}

QList<double> HArtist::getSimilarScores() {

    if(s_similarData.got) {
        QList<double> ret;
        for(int i=0;i<s_similarData.similar.size();i++) {
            ret.push_back(s_similarData.score[i].toDouble());
        }
        return ret;
    }
    s_similarData.getData(s_name);
    return getSimilarScores();
}

//////////////////////////////////////////////////////////////////////////////////////////////

static QPixmap download(QUrl url, bool tryAgain=1) {
    if(!url.isValid()) url="http://cdn.last.fm/flatness/catalogue/noimage/2/default_artist_large.png";
    QString t=QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/hathorMP";
    if(!QFile::exists(t)) {
        QDir r=QDir::root();
        r.mkpath(t);
    }
    QString x=url.toString();
    if(x.contains("png")) t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+".png";
    else t+="/"+QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+".jpg";
    if(!QFile::exists(t)) {
        QHttp http;
        QEventLoop loop;
        QFile file;
        QObject::connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));

        file.setFileName(t);
        file.open(QIODevice::WriteOnly);

        http.setHost(url.host(), url.port(80));
        http.get(url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority),
                 &file);
        loop.exec();
        file.close();
    }
    QPixmap apix;
    apix.load(t);
    if(!apix.width()&&tryAgain) { QFile::remove(t); download(url,0); }
    return apix;
}

HArtist::HArtist(QString name) : s_name(name)
{
}

void HArtist::PictureData::getData(QString url,HArtist::PictureSize size) {
    if(got[size]) {
        return;
    }
    got[size]=1;
    pics[size]=download(url);   //caches
    if(pics[size].isNull()) pics[size]=download(url);   //jic
    if(!pics[size].height()) {
        pics[size]=QPixmap(126,200);
        pics[size].fill(Qt::red);
    }
}

void HArtist::InfoData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","artistInfo");
    if(sett.value("cache for "+artist,0).toInt()==2) {
        pics[0]=sett.value("smallPic for "+artist).toString();
        pics[1]=sett.value("mediumPic for "+artist).toString();
        pics[2]=sett.value("largePic for "+artist).toString();
        pics[3]=sett.value("megaPic for "+artist).toString();

        bioShort=sett.value("bioShort for "+artist).toString();
        bio=sett.value("bio for "+artist).toString();
        tags=sett.value("tags for "+artist).toStringList();
        listenerCount=sett.value("listeners for "+artist).toInt();
        playCount=sett.value("playcount for "+artist).toInt();
        userPlayCount=sett.value("userplaycount for "+artist).toInt();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.exec();
    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
        return;
    }

    userPlayCount=0;

    try {
        QDomDocument doc;
        doc.setContent( reply->readAll() );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") pics[Small].append( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") pics[Medium].append( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") pics[Large].append( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="mega") pics[Mega].append( l.toText().data() );
                    }
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) bioShort = k.toText().data();
                        else if ( l.nodeName() == "content" ) bio = k.toText().data();
                        else if ( l.nodeName() == "listeners") listenerCount = k.toText().data().toInt();
                        else if ( l.nodeName() == "playcount") playCount = k.toText().data().toInt();
                        else if ( l.nodeName() == "userplaycount" ) userPlayCount = k.toText().data().toInt();
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
    bio.replace((char)13,"<br>");
    sett.setValue("cache for "+artist,2);
    sett.setValue("smallPic for "+artist,pics[0]);
    sett.setValue("mediumPic for "+artist,pics[1]);
    sett.setValue("largePic for "+artist,pics[2]);
    sett.setValue("megaPic for "+artist,pics[3]);
    sett.setValue("bioShort for "+artist,bioShort);
    sett.setValue("bio for "+artist,bio);
    sett.setValue("tags for "+artist,tags);
    sett.setValue("listeners for "+artist,listenerCount);
    sett.setValue("playcount for "+artist,playCount);
    sett.setValue("userplaycount for "+artist,userPlayCount);
}

void HArtist::AlbumData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","artistAlbums");
    if(sett.value("cache for "+artist,0).toInt()==2) {
        albums=sett.value("albums for "+artist).toStringList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getTopAlbums";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
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
                        if ( l.nodeName() == "name" ) albums.push_back( k.toText().data() );
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+artist,2);
    sett.setValue("albums for "+artist,albums);
}

void HArtist::ExtraTagData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","extraTagData");
    if(sett.value("cache for "+artist,0).toInt()==2) {
        tags=sett.value("tags for "+artist).toStringList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getTopTags";
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
        return;
    }

    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
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
                        if ( l.nodeName() == "name" ) tags.push_back( k.toText().data() );
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+artist,2);
    sett.setValue("tags for "+artist,tags);
}

void HArtist::TrackData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","artistTracks");
    if(sett.value("cache for "+artist,0).toInt()==2) {
        tracks=sett.value("tracks for "+artist).toStringList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getTopTracks";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );
    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
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
                        if ( l.nodeName() == "name" ) tracks.push_back( k.toText().data() );
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+artist,2);
    sett.setValue("tracks for "+artist,tracks);
}

void HArtist::SimilarData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","artistSimilar");
    if(sett.value("cache for "+artist,0).toInt()==2) {
        similar=sett.value("similar for "+artist).toStringList();
        score=sett.value("score for "+artist).toList();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "artist.getSimilar";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
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
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+artist,2);
    sett.setValue("similar for "+artist,similar);
    sett.setValue("score for "+artist,score);
}

void HArtist::ShoutData::getData(QString artist) {
    if(got) {
        return;
    }
    got=1;

    // no cache?

    QMap<QString, QString> params;
    params["method"] = "artist.getShouts";
    params["artist"] = artist;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist);
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
