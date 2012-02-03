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

QHash<QString, HAlbum*> HAlbum::_map;

HAlbum::HAlbum(QString artist, QString album) : s_artist(artist), s_album(album), s_albumInfo(artist,album), s_extraTagData(artist,album)
{
    for(int i=0;i<4;i++) s_cachedPixmap[i]=0;
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

void HAlbum::sendTrackNames(QObject *obj, const char *member) {
    s_albumInfo.sendProperty("trackNames",obj,member);
}

//QStringList HAlbum::getTrackNames() {
//    if(s_albumInfo.got) {
//        return s_albumInfo.tracks;
//    }
//    s_albumInfo.getData(s_artist,s_album);
//    return getTrackNames();
//}

void HAlbum::sendTracks(QObject *obj, const char *member) {
    s_trackQueue.push_back(qMakePair(obj,QString(member)));
    sendTrackNames(this,"sendTracks_2");
}

void HAlbum::sendTracks_2(QStringList tracks) {
    QMutexLocker locker(&queueMutex);
    QList<HTrack*> ret;
    for(int i=0;i<tracks.size();i++) {
        qDebug()<<"SEND TRACK"<<s_artist<<tracks[i];
        ret.push_back(&HTrack::get(s_artist,tracks[i]));
    }

    while(s_trackQueue.size()) {
        QPair< QObject*, QString > p=s_trackQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HTrack*>,ret));
    }
}

void HAlbum::sendPlayCount(QObject *obj, const char *member) { s_albumInfo.sendProperty("playCount",obj,member); }
void HAlbum::sendListenerCount(QObject *obj, const char *member) { s_albumInfo.sendProperty("listenerCount",obj,member); }
void HAlbum::sendUserPlayCount(QObject *obj, const char *member) { s_albumInfo.sendProperty("userPlayCount",obj,member); }
void HAlbum::sendPicNames(PictureSize size, QObject *obj, const char *member) { s_albumInfo.sendProperty("pics::"+QString::number(size),obj,member); }
void HAlbum::sendTagNames(QObject *obj, const char *member) { s_albumInfo.sendProperty("tagNames",obj,member); }
void HAlbum::sendMoreTagNames(QObject *obj, const char *member) { s_extraTagData.sendProperty("tagNames",obj,member); }
void HAlbum::sendSummary(QObject *obj, const char *member) { s_albumInfo.sendProperty("summary",obj,member); }
void HAlbum::sendContent(QObject *obj, const char *member) { s_albumInfo.sendProperty("content",obj,member); }

void HAlbum::sendPic(PictureSize p, QObject *obj, const char *member) {
    s_picQueue[p].push_back(qMakePair(obj,QString(member)));
    sendPicNames(p,this,QString("sendPic_2_"+QString::number(p)).toUtf8().data());
}

void HAlbum::sendPic_2(PictureSize p,QString pic) {
    if(!s_cachedPixmap[p]) s_cachedPixmap[p]=new HCachedPixmap(QUrl(pic));
    for(int i=0;i<s_picQueue[p].size();i++) {
        s_cachedPixmap[p]->send(s_picQueue[p][i].first,s_picQueue[p][i].second);
    }
    s_picQueue[p].clear();
}

void HAlbum::sendTags(QObject *obj, const char *member) {
    s_tagQueue.push_back(qMakePair(obj,QString(member)));
    sendTagNames(this,"sendTags_2");
}

void HAlbum::sendTags_2(QStringList t) {
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


void HAlbum::sendMoreTags(QObject *obj, const char *member) {
    s_moreTagQueue.push_back(qMakePair(obj,QString(member)));
    sendMoreTagNames(this,"sendMoreTags_2");
}

void HAlbum::sendMoreTags_2(QStringList t) {
    QList<HTag*> ret;
    for(int i=0;i<t.size();i++) {
        ret.push_back(&HTag::get(t[i]));
    }
    for(int i=0;i<s_moreTagQueue.size();i++) {
        QMetaObject::invokeMethod(s_moreTagQueue[i].first,s_moreTagQueue[i].second.toUtf8().data(),Q_ARG(QList<HTag*>,ret));
    }
    s_moreTagQueue.clear();
}


void HAlbum::sendShouts(QObject *obj, const char *member) {
    s_shoutData.shoutQueue.push_back(qMakePair(obj,QString(member)));
    s_shoutData.sendData(s_artist,s_album);
}

//////////////////////////////////////////////////////////////////////////////////////////////

AlbumInfo::AlbumInfo(QString artist, QString album) {
    QMap<QString, QString> params;
    params["method"] = "album.getInfo";
    params["username"] = lastfm::ws::Username;
    params["artist"] = artist;
    params["album"] = album;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+album.toUtf8()+"ALBUMINFO",QCryptographicHash::Md5).toHex();
    addProperty<int>("playCount",b);
    addProperty<int>("listenerCount",b);
    addProperty<int>("userPlayCount",b);
    addProperty<QString>("pics::0",b);
    addProperty<QString>("pics::1",b);
    addProperty<QString>("pics::2",b);
    addProperty<QString>("pics::3",b);
    addProperty<QStringList>("tagNames",b);
    addProperty<QStringList>("trackNames",b);
    addProperty<QString>("summary",b);
    addProperty<QString>("content",b);
}

bool AlbumInfo::process(const QString &d) {
    try {
        QDomDocument doc;
        doc.setContent( d );

        QDomElement element = doc.documentElement();

        QStringList tags, tracks;

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") setProperty("pics::0", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") setProperty("pics::1", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") setProperty("pics::2", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="mega") setProperty("pics::3", l.toText().data() );
                    }
                    else if ( m.nodeName() == "listeners") setProperty("listenerCount",l.toText().data().toInt());
                    else if ( m.nodeName() == "playcount") setProperty("playCount",l.toText().data().toInt());
                    else if ( m.nodeName() == "userplaycount" ) setProperty("userPlayCount", l.toText().data().toInt());

                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) setProperty("summary", HObject::eliminateHtml(k.toText().data()));
                        else if ( l.nodeName() == "content" ) setProperty("content", HObject::eliminateHtml(k.toText().data()));
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) tags.append( j.toText().data() );
                            if ( l.nodeName() == "track" && k.nodeName() == "name" ) tracks.append( j.toText().data() );
                        }
                    }
                }
            }
        }

        setProperty("tagNames",tags);
        setProperty("trackNames",tracks);
        return 1;

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
}

ExtraTagData::ExtraTagData(QString artist, QString album) {
    QMap<QString, QString> params;
    params["method"] = "album.getTopTags";
    params["artist"] = artist;
    params["album"] = album;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+album.toUtf8()+"EXTRATAGS",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("tagNames",b);
}

bool ExtraTagData::process(const QString &d) {
    try {
        QDomDocument doc;
        doc.setContent( d );
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
    }
    return 0;
}

void ShoutData::sendData(QString artist,QString album) {
    QMutexLocker lock(&mutex);  // DO NOT USE QMetaObject::invokeMethod
    this->artist=artist;
    this->album=album;

    if(getting) connect(getting,SIGNAL(notify()),this,SLOT(sendData_processQueue()));
    else if(got) sendData_processQueue();
    else {
        // no cache?
        QMap<QString, QString> params;
        params["method"] = "album.getShouts";
        params["artist"] = artist;
        params["album"] = album;
        QNetworkReply* reply = lastfmext_post( params );

        getting=new HRunOnceNotifier;   // !!
        connect(reply,SIGNAL(finished()),this,SLOT(sendData_process()));
    }
}

void ShoutData::sendData_process() {
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

void ShoutData::sendData_processQueue() {
    QMutexLocker locker(&mutex_2);
    while(shoutQueue.size()) {
        QPair< QObject*, QString > p=shoutQueue.takeFirst();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(QList<HShout*>,shouts));
    }
    shoutQueue.clear();
}
