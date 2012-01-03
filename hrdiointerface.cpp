#include "hrdiointerface.h"
#include "hauthaction.h"
#include <QEventLoop>
#include <QDebug>
#include <QDomDocument>
#include <QTimer>
#include <QSettings>
#include <stdexcept>
#include <qjson/parser.h>

#include "htrack.h"
#include "halbum.h"
#include "hartist.h"
#include "htoolbar.h"

HRdioInterface* HRdioInterface::_singleton=0;

HRdioInterface* HRdioInterface::login(QString username, QString password) {
    HRdioInterface* hri=new HRdioInterface(username,password);
    if(hri->ok()) {
        QSettings auth("hathorMP","auth");
        auth.setValue("rdio.token",hri->s_rdioToken);
        auth.setValue("rdio.secret",hri->s_rdioSecret);
        auth.setValue("rdio.oauthToken",hri->s_oauthToken);
        auth.setValue("rdio.oauthSecret",hri->s_oauthSecret);

        return _singleton=hri;
    }
    else {
        delete hri;
        return 0;
    }
}

HRdioInterface* HRdioInterface::restore() {
    QSettings auth("hathorMP","auth");
    HRdioInterface* hri=new HRdioInterface(auth.value("rdio.token").toString(),
                                           auth.value("rdio.secret").toString(),
                                           auth.value("rdio.oauthToken").toString(),
                                           auth.value("rdio.oauthSecret").toString());
    if(hri->ok()) {
        return _singleton=hri;
    } else {
        delete hri;
        return 0;
    }
}

HRdioInterface::HRdioInterface(QString username, QString password) :
    s_username(username), s_password(password), s_browser(), s_auth(new HAuthAction(s_browser,username,password)), s_ready(0)
{
    connect(s_auth,SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)),this,SLOT(oauth(QByteArray,QByteArray,QByteArray,QByteArray)));

    QEventLoop loop;
    loop.connect(s_auth, SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)), SLOT(quit()) );
    loop.connect(s_auth, SIGNAL(error(QString)), SLOT(quit()) );
    loop.exec();
}

HRdioInterface::HRdioInterface(QString rdioToken, QString rdioSecret, QString oauthToken, QString oauthSecret) :
    s_rdioToken(rdioToken.toUtf8()), s_rdioSecret(rdioSecret.toUtf8()), s_oauthToken(oauthToken.toUtf8()), s_oauthSecret(oauthSecret.toUtf8()), s_browser(), s_auth(0), s_ready(0)
{
    oauth(s_rdioToken,s_rdioSecret,s_oauthToken,s_oauthSecret);
}


bool HRdioInterface::ok() {
    return s_rdioToken.size()&&s_rdioSecret.size()&&s_oauthToken.size()&&s_oauthSecret.size();
}

void HRdioInterface::oauth(QByteArray rdioToken, QByteArray rdioSecret, QByteArray oauthToken,QByteArray oauthSecret) {
    s_rdioToken=rdioToken;
    s_rdioSecret=rdioSecret;
    s_oauthToken=oauthToken;
    s_oauthSecret=oauthSecret;

    QMultiMap<QByteArray,QByteArray> map1;
    map1.insert("method", "getPlaybackToken");
    map1.insert("domain", QUrl::toPercentEncoding("shdw.x10.mx"));
    map1.insert("format", QUrl::toPercentEncoding("xml"));
    //    map1.insert("query", QUrl::toPercentEncoding("Just for now"));
    //    map1.insert("types", "\"Track\"");
    QMultiMap<QByteArray,QByteArray> p=HBrowser::request(RDIO_CONSUMER_KEY,RDIO_CONSUMER_SECRET,"http://api.rdio.com/1/",map1,s_oauthToken,s_oauthSecret);
    if(p.values().size()<1) {
        qDebug()<<"ERR! Invalid response from rdio api!";
        return;
    }

    try {
        QDomDocument doc;
        QString x=p.values()[0];
        x.remove(0,x.indexOf(">")+1);
        doc.setContent( x );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                if(n.attributes().namedItem("name").nodeValue()=="result") s_playbackKey=m.toText().data();
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    if(s_playbackKey=="") {
        qDebug()<<"ERR! No playback key!";
    }
    // I SWEARZZ THIS IZNT SHADY!!!!!
    s_browser.loadPage("http://shdw.x10.mx/cgi-bin/rgen.perl?"+QUrl::toPercentEncoding(s_playbackKey));
    connect(s_browser.s_webView->page(),SIGNAL(alert(const QString&)),this,SLOT(jsCallback(QString)));
}

void HRdioInterface::setupPlayback() {
    QTimer::singleShot(2000,this,SLOT(tryPlay()));
}

void HRdioInterface::tryPlay() {
    s_browser.doJS("$('#api').rdio().play()");
}

void HRdioInterface::play(bool a) {
    if(a) tryPlay();
    else pause();
}

void HRdioInterface::pause() {
    s_browser.doJS("$('#api').rdio().pause()");
}

void HRdioInterface::next() {
    s_browser.doJS("$('#api').rdio().next()");
}

void HRdioInterface::superNext() {
    s_browser.doJS("$('#api').rdio().next(true)");
}

void HRdioInterface::prev() {
    s_browser.doJS("$('#api').rdio().previous()");
}

void HRdioInterface::seek(int sec) {
    s_browser.doJS("$('#api').rdio().seek("+QString::number(sec)+")");
}

void HRdioInterface::setShuffle(bool a) {
    s_browser.doJS("$('#api').rdio().setShuffle("+QString(a?"true":"false")+")");
}

void HRdioInterface::queue(HArtist& artist) {
    s_browser.doJS("$('#api').rdio().embed.rdio_queue('"+search(artist.getName(),"Artists")+"')");
    setupPlayback();
}

void HRdioInterface::queue(HAlbum& album) {
    QString aCode=search(album.getArtistName()+" "+album.getAlbumName(),"Albums");
    s_browser.doJS("$('#api').rdio().embed.rdio_queue('"+aCode+"')");
    setupPlayback();
}

void HRdioInterface::queue(HTrack& track) {
    s_browser.doJS("$('#api').rdio().embed.rdio_queue('"+search(track.getArtistName()+" "+track.getTrackName(),"Tracks")+"')");
    setupPlayback();

    if(s_state==Stopped) {
        QEventLoop loop;
        loop.connect( this, SIGNAL(playingTrackChanged(HTrack&)), SLOT(quit()) );
        loop.exec();
    }
}

void HRdioInterface::play(HArtist& artist) {
    s_browser.doJS("$('#api').rdio().play('"+search(artist.getName(),"Artists")+"')");
}

void HRdioInterface::play(HAlbum& album) {
    QString aCode=search(album.getArtistName()+" "+album.getAlbumName(),"Albums");
    s_browser.doJS("$('#api').rdio().play('"+aCode+"')");
}

void HRdioInterface::play(HTrack& track) {
    s_browser.doJS("$('#api').rdio().play('"+search(track.getArtistName()+" "+track.getTrackName(),"Tracks")+"')");

    if(s_state==Stopped) {
        QEventLoop loop;
        loop.connect( this, SIGNAL(playingTrackChanged(HTrack&)), SLOT(quit()) );
        loop.exec();
    }
}

void HRdioInterface::setVol(double v) {
    s_browser.doJS("$('#api').rdio().setVolume("+QString::number(v)+")");
}

void HRdioInterface::setMute(bool a) {
    s_browser.doJS("$('#api').rdio().setMute("+QString(a?"true":"false")+")");
}

void HRdioInterface::goToPosInQueue(int t) {
    s_browser.doJS("$('#api').rdio().playQueuedTrack("+QString::number(t)+")");
}

void HRdioInterface::clearQueue() {
    s_browser.doJS("$('#api').rdio().clearQueue()");
}

void HRdioInterface::jsCallback(QString cb) {
    qDebug()<<"Rdio callback:"<<cb;
    if(cb=="ready()") {
        s_ready=1;
        qDebug()<<"Ready to go!";

        return;
    }
    if(cb.startsWith("rdio.playingTrackChanged(")) {
        cb.remove("rdio.playingTrackChanged(");
        cb.chop(3);
        QJson::Parser parser;
        bool ok;
        QMap<QString, QVariant> result = parser.parse(cb.toUtf8(),&ok).toMap();
        if(!ok) {
            qDebug()<<"Problem parsing rdio.playingTrackChanged(...)";
            return;
        }
        s_currentInfo.duration=result["duration"].toInt();
        s_currentInfo.clean=(result["is_clean"].toString()=="true");
        s_currentInfo.arist=result["artist"].toString();
        s_currentInfo.album=result["album"].toString();
        s_currentInfo.expl=(result["is_explicit"].toString()=="true");
        s_currentInfo.name=result["name"].toString();
        s_currentInfo.valid=true;
        emit playingTrackChanged(HTrack::get(s_currentInfo.arist,s_currentInfo.name));
        HToolbar::singleton()->setPlaybackStatus("<A href=\"more\">"+s_currentInfo.name+" by "+s_currentInfo.arist);
        return;
    }
    if(cb.startsWith("rdio.playStateChanged(")) {
        cb.remove("rdio.playStateChanged(");
        cb.chop(1);
        s_state=(HRdioInterface::State)cb.toInt();
        emit playStateChanged(s_state);
        switch(s_state) {
        case Paused:
        case PausedB:
            HToolbar::singleton()->setPlaybackStatus("Paused");
            HToolbar::singleton()->setPlayChecked(false);
            HToolbar::singleton()->setPlayEnabled(true);
            break;
        case Stopped:
            HToolbar::singleton()->setPlaybackStatus("Stopped");
            HToolbar::singleton()->setPlayChecked(false);
            HToolbar::singleton()->setPlayEnabled(false);
            break;
        case Buffering:
            HToolbar::singleton()->setPlaybackStatus("Buffering...");
            HToolbar::singleton()->setPlayChecked(true);
            HToolbar::singleton()->setPlayEnabled(true);
            break;
        case Playing:
            HToolbar::singleton()->setPlaybackStatus("<A href=\"more\">"+s_currentInfo.name+" by "+s_currentInfo.arist);
            HToolbar::singleton()->setPlayChecked(true);
            HToolbar::singleton()->setPlayEnabled(true);
            break;
        }

        return;
    }
    if(cb.startsWith("rdio.positionChanged(")) {
        cb.remove("rdio.positionChanged(");
        cb.chop(1);
        s_curPos=cb.toDouble();
        emit positionChanged(s_curPos);
        return;
    }
}

void HRdioInterface::keshaTest() {
    QString f="$('#api').rdio().embed.rdio_queue('"+search("Ke$ha","Artists")+"')";
    qDebug()<<"About to run f:"<<f;
    s_browser.doJS(f);
    setupPlayback();
}

QString HRdioInterface::search(QString search,QString types) {  //no auth!
    QMultiMap<QByteArray,QByteArray> map1;
    map1.insert("method", "search");
    map1.insert("query", QUrl::toPercentEncoding(search));
    map1.insert("types", QUrl::toPercentEncoding(types));
    map1.insert("format", QUrl::toPercentEncoding("xml"));
    QMultiMap<QByteArray,QByteArray> p=HBrowser::request(RDIO_CONSUMER_KEY,RDIO_CONSUMER_SECRET,"http://api.rdio.com/1/",map1);
    if(p.values().size()<1) {
        qDebug()<<"ERR! Invalid response from rdio api!";
        return "";
    }

    try {
        QDomDocument doc;
        QString x=p.values()[0];
        x.remove(0,x.indexOf(">")+1);
        doc.setContent( x );

        QDomElement element = doc.documentElement();

        bool ok=1;
        int checked=0;
        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if(k.attributes().namedItem("name").nodeValue()=="key"&&(ok||checked>=2)) {
                                QString strx=j.toText().data();
                                if(strx.size()&&strx[0]=='r') strx='t'+strx;

                                return strx;
                            }
                            if(k.attributes().namedItem("name").nodeValue()=="canStream") {
                                qDebug()<<"canStream:"<<j.toText().data();
                                ok=(j.toText().data()=="true");
                                ++checked;
                            }
                        }

                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    qWarning()<<"Could not find best search!";
    return "";
}
