#include "huser.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QEventLoop>
#include <QSettings>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"

QMap<QString, HUser*> HUser::_map;

HUser::HUser(QString username) : s_username(username)
{
}

HUser& HUser::get(QString username) {
    if(_map.value(username,0)) {
        return *_map.value(username);
    }

    _map.insert(username,new HUser(username));
    return get(username);
}

QString HUser::getRealName() {
    if(s_infoData.got) {
        return s_infoData.realName;
    }
    s_infoData.getData(s_username);
    return getRealName();
}

QPixmap HUser::getPic(PictureSize p) {
    if(s_pictureData.got[p]) return s_pictureData.pics[p];
    if(s_infoData.got) {
        while(s_infoData.pics[p].isEmpty()&&p) p=(PictureSize)((int)p-1);
        s_pictureData.getData(s_infoData.pics[p],p);
        return getPic(p);
    }
    s_infoData.getData(s_username);
    return getPic(p);
}

int HUser::getAge() {
    if(s_infoData.got) {
        return s_infoData.age;
    }
    s_infoData.getData(s_username);
    return getAge();
}

bool HUser::getMale() {
    if(s_infoData.got) {
        return s_infoData.male;
    }
    s_infoData.getData(s_username);
    return getMale();
}

int HUser::getPlayCount() {
    if(s_infoData.got) {
        return s_infoData.playCount;
    }
    s_infoData.getData(s_username);
    return getPlayCount();
}

int HUser::getPlaylists() {
    if(s_infoData.got) {
        return s_infoData.playlists;
    }
    s_infoData.getData(s_username);
    return getPlaylists();
}

//////////////////////////////////////////////////////////////////////////////////////////////


static QPixmap download(const QUrl &url) {
    QString t=QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/hathorMP";
    if(!QFile::exists(t)) {
        QDir r=QDir::root();
        r.mkpath(t);
    }
    QString x=url.toString();
    if(x.contains("png")) t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+".png";
    else if(x.contains("gif")) t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+".gif";
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
    return apix;
}

void HUser::PictureData::getData(QString url,HUser::PictureSize size) {
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

void HUser::InfoData::getData(QString username) {
    if(got) {
        return;
    }
    got=1;

    QSettings sett("hathorMP","userInfo");

    if(sett.value("cache for "+username,0).toInt()==2) {
        realName=sett.value("realName "+username).toString();

        pics[0]=sett.value("smallPic for "+username).toString();
        pics[1]=sett.value("mediumPic for "+username).toString();
        pics[2]=sett.value("largePic for "+username).toString();
        pics[3]=sett.value("megaPic for "+username).toString();

        country=sett.value("country for "+username).toString();
        age=sett.value("age for "+username).toInt();
        male=sett.value("male for "+username).toBool();
        playCount=sett.value("playcount for "+username).toInt();
        playlists=sett.value("playlists for "+username).toInt();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "user.getInfo";
    params["user"] = username;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(reply->error()!=QNetworkReply::NoError) {
        got=0;
        getData(username);
        return;
    }
    /*
        QString realName;
        QString pics[4];
        QString country;
        int age;
        bool male;
        int playCount;
        int playlists;
        bool got;
        */
    male=0;
    playCount=0;
    playlists=0;
    age=0;
    try {
        QDomDocument doc;
        doc.setContent( reply->readAll() );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "image") {
                        if(m.attributes().namedItem("size").nodeValue()=="small") pics[Small]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="medium") pics[Medium]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="large") pics[Large]=( l.toText().data() );
                        else if(m.attributes().namedItem("size").nodeValue()=="extralarge") pics[Mega]=( l.toText().data() );   //not mega!
                    }
                    else if ( m.nodeName() == "name") realName = l.toText().data();
                    else if ( m.nodeName() == "country") country = l.toText().data();
                    else if ( m.nodeName() == "age" ) age = l.toText().data().toInt();
                    else if ( m.nodeName() == "gender" ) male = ((l.toText().data()=="m")?1:0);
                    else if ( m.nodeName() == "playcount" ) playCount = l.toText().data().toInt();
                    else if ( m.nodeName() == "playlists" ) playlists = l.toText().data().toInt();
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+username,2);
    sett.setValue("realName "+username,realName);

    sett.setValue("smallPic for "+username,pics[0]);
    sett.setValue("mediumPic for "+username,pics[1]);
    sett.setValue("largePic for "+username,pics[2]);
    sett.setValue("megaPic for "+username,pics[3]);

    sett.setValue("country for "+username,country);
    sett.setValue("age for "+username,age);
    sett.setValue("male for "+username,male);
    sett.setValue("playcount for "+username,playCount);
    sett.setValue("playlists for "+username,playlists);
}
