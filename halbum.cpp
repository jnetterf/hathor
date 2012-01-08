#include "halbum.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QDomDocument>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QEventLoop>
#include <QSettings>
#include "hartist.h"
#include <lastfm/ws.h>
#include "lastfmext.h"

QMap<QString, HAlbum*> HAlbum::_map;

HAlbum::HAlbum(QString artist, QString album) : s_artist(artist), s_album(album)
{
}

HAlbum& HAlbum::get(QString artist, QString album) {
    if(_map.value(artist+"__"+album,0)) {
        return *_map.value(artist+"__"+album);
    }
    
    _map.insert(artist+"__"+album,new HAlbum(artist,album));
    return get(artist,album);
}

HArtist& HAlbum::getArtist() {
    return HArtist::get(s_artist);
}

QList<HTrack*> HAlbum::getTracks() {
    QStringList tracks=getTrackNames();
    QList<HTrack*> ret;
    for(int i=0;i<tracks.size();i++) {
        ret.push_back(&HTrack::get(s_artist,tracks[i]));
    }
    return ret;
}

QStringList HAlbum::getTrackNames() {
    if(s_albumInfo.got) {
        return s_albumInfo.tracks;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getTrackNames();
}

int HAlbum::getPlayCount() {
    if(s_albumInfo.got) {
        return s_albumInfo.playCount;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getPlayCount();
}

int HAlbum::getListenerCount() {
    if(s_albumInfo.got) {
        return s_albumInfo.listenerCount;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getListenerCount();
}

int HAlbum::getUserPlayCount() {
    if(s_albumInfo.got) {
        return s_albumInfo.userPlayCount;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getUserPlayCount();
}

QPixmap HAlbum::getPic(PictureSize p) {
    if(s_pictureData.got[p]) return s_pictureData.pics[p];
    if(s_albumInfo.got) {
        while(s_albumInfo.pics[p].isEmpty()&&p) p=(PictureSize)((int)p-1);
        s_pictureData.getData(s_albumInfo.pics[p],p);
        return getPic(p);
    }
    s_albumInfo.getData(s_artist,s_album);
    return getPic(p);
}

QStringList HAlbum::getTagNames() {
    if(s_albumInfo.got) {
        return s_albumInfo.tags;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getTagNames();
}

QList<HTag*> HAlbum::getTags() {
    QStringList tags=getTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}

QList<HTag*> HAlbum::getMoreTags() {
    QStringList tags=getMoreTagNames();
    QList<HTag*> ret;
    for(int i=0;i<tags.size();i++) {
        ret.push_back(&HTag::get(tags[i]));
    }
    return ret;
}

QStringList HAlbum::getMoreTagNames() {
    if(s_extraTagData.got) {
        return s_extraTagData.tags;
    }
    s_extraTagData.getData(s_artist,s_album);
    return getMoreTagNames();
}

QString HAlbum::getSummary() {
    if(s_albumInfo.got) {
        return s_albumInfo.summary;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getSummary();
}

QString HAlbum::getContent() {
    if(s_albumInfo.got) {
        return s_albumInfo.content;
    }
    s_albumInfo.getData(s_artist,s_album);
    return getContent();
}

QList<HShout*> HAlbum::getShouts() {
    if(s_shoutData.got) return s_shoutData.shouts;
    s_shoutData.getData(s_artist,s_album);
    return getShouts();
}

//////////////////////////////////////////////////////////////////////////////////////////////


static QPixmap download(QUrl url, bool tryAgain=1) {
    if(!url.isValid()) url="http://cdn.last.fm/flatness/catalogue/noimage/2/default_artist_mega.png";
    QString t=QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/hathorMP";
    if(!QFile::exists(t)) {
        QDir r=QDir::root();
        r.mkpath(t);
    }
    QString x=url.toString();
    QString y=x;
    y.remove(0,y.lastIndexOf('.'));
    t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+y;
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

void HAlbum::PictureData::getData(QString url,HAlbum::PictureSize size) {
    if(getting[size]) {
        QEventLoop ev;
        connect(getting[size],SIGNAL(notify()),&ev,SLOT(quit()));
        ev.exec();
        return;
    }


    if(got[size]) {
        return;
    }

    getting[size]=new HRunOnceNotifier;
    pics[size]=download(url);   //caches
    if(pics[size].isNull()) pics[size]=download(url);   //jic
    if(!pics[size].height()) {
        pics[size]=QPixmap(126,200);
        pics[size].fill(Qt::red);
    }

    getting[size]->emitNotify();
    getting[size]=0;

    got[size]=1;
}

void HAlbum::AlbumInfo::getData(QString artist,QString album) {
    H_BEGIN_RUN_ONCE;

    QSettings sett("hathorMP","albumInfo");
    QString dumbName=artist+"__"+album;
    if(sett.value("cache for "+dumbName,0).toInt()==3) {
        pics[0]=sett.value("smallPic for "+dumbName).toString();
        pics[1]=sett.value("mediumPic for "+dumbName).toString();
        pics[2]=sett.value("largePic for "+dumbName).toString();
        pics[3]=sett.value("megaPic for "+dumbName).toString();
        
        tags=sett.value("tags for "+dumbName).toStringList();
        listenerCount=sett.value("listeners for "+dumbName).toInt();
        playCount=sett.value("playcount for "+dumbName).toInt();
        userPlayCount=sett.value("userplaycount for "+dumbName).toInt();
        summary=sett.value("summary for "+dumbName).toString();
        content=sett.value("content for "+dumbName).toString();
        tracks=sett.value("tracks for "+dumbName).toStringList();
        H_END_RUN_ONCE;
        return;
    }
    
    QMap<QString, QString> params;
    params["method"] = "album.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["album"] = album;
    QNetworkReply* reply = lastfmext_post( params );
    
    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();
    
    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        H_END_RUN_ONCE;
        QByteArray x=reply->readAll();
        if(x.contains("error")) {
            return;
        }
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist,album);
        return;
    }
    
    listenerCount=0;
    playCount=0;
    userPlayCount=0;
    summary="";
    content="";
    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();
        
        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") pics[Small]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") pics[Medium]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") pics[Large]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="mega") pics[Mega]=( l.toText().data() );
                    }
                    else if ( m.nodeName() == "listeners") listenerCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "playcount") playCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "userplaycount" ) userPlayCount = l.toText().data().toInt();
                    
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) summary = k.toText().data();
                        else if ( l.nodeName() == "content" ) content = k.toText().data();
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) tags.append( j.toText().data() );
                            if ( l.nodeName() == "track" && k.nodeName() == "name" ) tracks.append( j.toText().data() );
                        }
                    }
                }
            }
        }
        
    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    content.replace((char)13,"<br>");
    sett.setValue("cache for "+dumbName,3);
    sett.setValue("smallPic for "+dumbName,pics[0]);
    sett.setValue("mediumPic for "+dumbName,pics[1]);
    sett.setValue("largePic for "+dumbName,pics[2]);
    sett.setValue("megaPic for "+dumbName,pics[3]);
    sett.setValue("tags for "+dumbName,tags);
    sett.setValue("listeners for "+dumbName,listenerCount);
    sett.setValue("playcount for "+dumbName,playCount);
    sett.setValue("userplaycount for "+dumbName,userPlayCount);
    sett.setValue("summary for "+dumbName,summary);
    sett.setValue("content for "+dumbName,content);
    sett.setValue("tracks for "+dumbName,tracks);
    H_END_RUN_ONCE;
}


void HAlbum::ExtraTagData::getData(QString artist,QString album) {
    H_BEGIN_RUN_ONCE;
    
    QString dumbname=artist+"__"+album;
    
    QSettings sett("hathorMP","extraTagData");
    if(sett.value("cache for "+dumbname,0).toInt()==2) {
        tags=sett.value("tags for "+dumbname).toStringList();
        H_END_RUN_ONCE;
        return;
    }
    
    QMap<QString, QString> params;
    params["method"] = "album.getTopTags";
    params["artist"] = artist;
    params["album"] = album;
    QNetworkReply* reply = lastfmext_post( params );
    
    if(reply->error()!=QNetworkReply::NoError) {
        H_END_RUN_ONCE;
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist,album);
        return;
    }
    
    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();
    
    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

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
    sett.setValue("cache for "+dumbname,2);
    sett.setValue("tags for "+dumbname,tags);
    H_END_RUN_ONCE;
}

void HAlbum::ShoutData::getData(QString artist,QString album) {
    H_BEGIN_RUN_ONCE;
    
    // no cache?
    
    QMap<QString, QString> params;
    params["method"] = "album.getShouts";
    params["artist"] = artist;
    params["album"] = album;
    QNetworkReply* reply = lastfmext_post( params );
    
    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();
    
    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        H_END_RUN_ONCE;
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(artist,album);
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
    H_END_RUN_ONCE;
}
