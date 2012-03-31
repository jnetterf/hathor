#include "halbum.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QDomDocument>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QSettings>
#include "hartist.h"
#include <lastfm/ws.h>
#include "lastfmext.h"

QHash<QString, HAlbum*> HAlbum::_map;

HAlbum::HAlbum(QString artist, QString album) : s_artist(artist), s_album(album), s_gotShouts(0), s_shoutData(artist,album), s_albumInfo(artist,album), s_extraTagData(artist,album)
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

int** HAlbum::sendTrackNames(QObject *obj, const char *member, QObject *guest) {
    return s_albumInfo.sendProperty("trackNames",obj,member,guest);
}

//QStringList HAlbum::getTrackNames() {
//    if(s_albumInfo.got) {
//        return s_albumInfo.tracks;
//    }
//    s_albumInfo.getData(s_artist,s_album);
//    return getTrackNames();
//}

int** HAlbum::sendTracks(QObject *obj, const char *member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_trackQueue.push_back(qMakePair(obj,QString(member)));
    return sendTrackNames(this,"sendTracks_2",obj);
}

void HAlbum::sendTracks_2(QStringList tracks) {
    while(s_trackQueue.size()) {
        QPair< QObject*, QString > p=s_trackQueue.takeFirst();
        for(int i=0;i<tracks.size();i++) {
            QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HTrack*,&HTrack::get(s_artist,tracks[i])));
        }
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HTrack*,0));
    }
}

int** HAlbum::sendPlayCount(QObject *obj, const char *member) { return s_albumInfo.sendProperty("playCount",obj,member); }
int** HAlbum::sendListenerCount(QObject *obj, const char *member) { return s_albumInfo.sendProperty("listenerCount",obj,member); }
int** HAlbum::sendUserPlayCount(QObject *obj, const char *member) { return s_albumInfo.sendProperty("userPlayCount",obj,member); }
int** HAlbum::sendPicNames(PictureSize size, QObject *obj, const char *member, QObject* guest) { return s_albumInfo.sendProperty("pics::"+QString::number(size),obj,member,guest); }
int** HAlbum::sendTagNames(QObject *obj, const char *member) { return s_albumInfo.sendProperty("tagNames",obj,member); }
int** HAlbum::sendMoreTagNames(QObject *obj, const char *member,QObject* guest) { return s_extraTagData.sendProperty("tagNames",obj,member,guest); }
int** HAlbum::sendSummary(QObject *obj, const char *member) { return s_albumInfo.sendProperty("summary",obj,member); }
int** HAlbum::sendContent(QObject *obj, const char *member) { return s_albumInfo.sendProperty("content",obj,member); }

int** HAlbum::sendPic(PictureSize p, QObject *obj, const char *member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_picQueue[p].push_back(qMakePair(obj,QString(member)));
    return sendPicNames(p,this,QString("sendPic_2_"+QString::number(p)).toUtf8().data(),obj);
}

void HAlbum::sendPic_2(PictureSize p,QString pic) {
    if(!s_cachedPixmap[p]) s_cachedPixmap[p]=HCachedPixmap::get(QUrl(pic));
    for(int i=0;i<s_picQueue[p].size();i++) {
        *s_cachedPixmap[p]->send(s_picQueue[p][i].first,s_picQueue[p][i].second)=*s_albumInfo.getPriorityForProperty(s_picQueue[p][i].first,"pics::"+QString::number(p));
//        qDebug()<<">>>"<<**s_cachedPixmap[p]->send(s_picQueue[p][i].first,s_picQueue[p][i].second);
    }
    s_picQueue[p].clear();
}

int** HAlbum::sendTags(QObject *obj, const char *member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_tagQueue.push_back(qMakePair(obj,QString(member)));
    return sendTagNames(this,"sendTags_2");
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


int** HAlbum::sendMoreTags(QObject *obj, const char *member) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_moreTagQueue.push_back(qMakePair(obj,QString(member)));
    return sendMoreTagNames(this,"sendMoreTags_2",obj);
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

int** HAlbum::sendShouts(QObject *obj, QString member,int count) {
    connect(obj,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
    s_shoutQueue.push_back(HTriple(obj,member,count));
    return &(*s_shoutData.sendProperty("shouts",this,"sendShouts_2",obj)=*s_shoutData.sendProperty("authors",this,"sendShouts_3",obj)=*s_shoutData.sendProperty("dates",this,"sendShouts_4",obj));
}

void HAlbum::sendShouts_2(QStringList t) {
    s_gotShouts=1;
    s_shout_shouts=t;
    sendShouts_4(QStringList());
}

void HAlbum::sendShouts_3(QStringList t) {
    s_shout_artists=t;
    sendShouts_4(QStringList());
}

void HAlbum::sendShouts_4(QStringList t) {
    if(!s_shout_dates.size()) s_shout_dates=t;
    t=s_shout_dates;
    if(s_gotShouts&&(s_shout_shouts.size()==s_shout_artists.size())&&(s_shout_shouts.size()==s_shout_dates.size())) {
        for(int i=0;i<t.size();i++) {
            bool ok=0;
            for(int j=0;j<s_shoutQueue.size();j++) {
                if((i<s_shoutQueue[j].third)||(s_shoutQueue[j].third==-1)) {
                    ok=1;
                    if(i==shouts.size()) {
                        shouts.push_back(new HShout(s_shout_shouts[i],HUser::get(s_shout_artists[i]),s_shout_dates[i]));
                    }
                    QMetaObject::invokeMethod(s_shoutQueue[j].first,s_shoutQueue[j].second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HShout*,shouts[i]));
                }
            }
            if(!ok) break;
        }
        for(int i=0;i<s_shoutQueue.size();i++) {
            QMetaObject::invokeMethod(s_shoutQueue[i].first,s_shoutQueue[i].second.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HShout*,0));
        }
        s_shoutQueue.clear();
    }
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
                        if(m.attributes().namedItem("size").nodeValue()=="small"&& l.toText().data().size()) setProperty("pics::0", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium"&& l.toText().data().size()) setProperty("pics::1", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large"&& l.toText().data().size()) setProperty("pics::2", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="mega"&& l.toText().data().size()) setProperty("pics::3", l.toText().data() );
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

HAlbum::ShoutData::ShoutData(QString artist,QString album) {
    QMap<QString, QString> params;
    params["method"] = "album.getShouts";
    params["artist"] = artist;
    params["album"] = album;
    setParams(params);
    QByteArray b=QCryptographicHash::hash(artist.toUtf8()+album.toUtf8()+"SHOUTDATA",QCryptographicHash::Md5).toHex();
    addProperty<QStringList>("shouts",b);
    addProperty<QStringList>("authors",b);
    addProperty<QStringList>("dates",b);
}

bool HAlbum::ShoutData::process(const QString &data) {
    QStringList shouts, authors, dates;
    try {
        QDomDocument doc;
        doc.setContent( data );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "body" ) shouts.push_back(k.toText().data());
                        else if ( l.nodeName() == "author" ) authors.push_back(k.toText().data());
                        else if ( l.nodeName() == "date") {
                            dates.push_back(k.toText().data());
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    setProperty("shouts",shouts);
    setProperty("authors",authors);
    setProperty("dates",dates);
    return 1;
}
