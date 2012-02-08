#include "huser.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QSettings>
#include <QTimer>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"

QHash<QString, HUser*> HUser::_map;

HUser::HUser(QString username) : s_username(username), s_infoData(username)
{
    for(int i=0;i<4;i++) s_cachedPixmap[i]=0;
}

HUser& HUser::get(QString username) {
    if(_map.value(username,0)) {
        return *_map.value(username);
    }

    _map.insert(username,new HUser(username));
    return get(username);
}

void HUser::sendRealName(QObject* o, QString m) {
    s_infoData.sendProperty("realName",o,m);
}

void HUser::sendPicNames(PictureSize p, QObject* obj, QString member) {
    s_infoData.sendProperty("pic_"+QString::number(p),obj,member);
}

void HUser::sendPic(PictureSize p, QObject* obj, QString member) {
    s_picQueue[p].push_back(qMakePair(obj,QString(member)));
    sendPicNames(p,this,QString("sendPic_2_"+QString::number(p)).toUtf8().data());
}

void HUser::sendPic_2(PictureSize p,QString pic) {
    if(!s_cachedPixmap[p]) s_cachedPixmap[p]=HCachedPixmap::get(QUrl(pic));
    for(int i=0;i<s_picQueue[p].size();i++) {
        s_cachedPixmap[p]->send(s_picQueue[p][i].first,s_picQueue[p][i].second);
    }
    s_picQueue[p].clear();
}

void HUser::sendAge(QObject* o, QString m) {
    s_infoData.sendProperty("age",o,m);
}

void HUser::sendGender(QObject* o, QString m) {
    s_infoData.sendProperty("gender",o,m);
}

void HUser::sendPlayCount(QObject* o, QString m) {
    s_infoData.sendProperty("playCount",o,m);
}

void HUser::sendPlaylists(QObject* o, QString m) {
    s_infoData.sendProperty("playlistCount",o,m);
}

//QList<HTrack*> HUser::sendTopTracks() {
//    if(s_topTrackData.got) {
//        return s_topTrackData.topTracks;
//    }
//    s_topTrackData.getData(s_username);
//    return getTopTracks();
//}

//////////////////////////////////////////////////////////////////////////////////////////////

HUser::InfoData::InfoData(QString username) {
    QMap<QString, QString> params;
    params["method"] = "user.getInfo";
    params["user"] = username;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(username.toUtf8()+"USERINFO",QCryptographicHash::Md5).toHex();
    addProperty<QString>("realName",b);

    addProperty<QString>("pic_0",b);
    addProperty<QString>("pic_1",b);
    addProperty<QString>("pic_2",b);
    addProperty<QString>("pic_3",b);

    addProperty<QString>("country",b);
    addProperty<int>("age",b);
    addProperty<QString>("gender",b);
    addProperty<int>("playCount",b);
    addProperty<int>("playlistCount",b);
}

bool HUser::InfoData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );

        QDomElement element = doc.documentElement();
        setProperty<QString>("gender","unknown");

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") setProperty( "pic_0", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") setProperty( "pic_1", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") setProperty( "pic_2", l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="extralarge") setProperty( "pic_3", l.toText().data() );   //not mega!
                    }
                    else if ( m.nodeName() == "name") setProperty("realName", l.toText().data());
                    else if ( m.nodeName() == "country") setProperty("country",l.toText().data());
                    else if ( m.nodeName() == "age" ) setProperty("age",l.toText().data().toInt());
                    else if ( m.nodeName() == "gender" ) setProperty<QString>("gender",((l.toText().data()=="m")?"male":"female"));
                    else if ( m.nodeName() == "playcount" ) setProperty("playCount",l.toText().data().toInt());
                    else if ( m.nodeName() == "playlists" ) setProperty("playlistCount",l.toText().data().toInt());
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}

//void HUser::TopTrackData::getData(QString username) {
//    if(got) {
//        return;
//    }
//    got=1;

//    QSettings sett("Nettek","Hathor_topTrackData");

//    if(sett.value("cache for "+username,0).toInt()==2) {
//        QStringList names, artists;
//        names=sett.value("topTrackNames for "+username).toStringList();
//        artists=sett.value("topTrackArtists for "+username).toStringList();
//        for(int i=0;i<names.size()&&i<artists.size();i++) {
//            topTracks.push_back(&HTrack::get(artists[i],names[i]));
//        }
//        return;
//    }

//    QMap<QString, QString> params;
//    params["method"] = "user.getTopTracks";
//    params["user"] = username;
//    params["period"] = "3month";
//    QNetworkReply* reply = lastfmext_post( params );

//    QTimer::singleShot(2850,&loop,SLOT(quit()));
//    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
//    loop.exec();

//    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
//        got=0;
//        getData(username);
//        return;
//    }

//    QStringList names,artists;
//    try {
//        QDomDocument doc;
//        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

//        QDomElement element = doc.documentElement();

//        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
//            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
//                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
//                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
//                        if ( l.nodeName() == "name" ) names.push_back(k.toText().data());
//                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
//                            if ( k.nodeName() == "name" ) {
//                                artists.push_back(j.toText().data());
//                                topTracks.push_back(&HTrack::get(artists.back(),names.back()));
//                            }
//                        }
//                    }
//                }
//            }
//        }

//    } catch (std::runtime_error& e) {
//        qWarning() << e.what();
//    }
//    sett.setValue("cache for "+username,2);

//    sett.setValue("topTrackNames for "+username,names);
//    sett.setValue("topTrackArtists for "+username,artists);
//}

