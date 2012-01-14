#include "hrdiointerface.h"
#include "hauthaction.h"
#include <QEventLoop>
#include <QDebug>
#include <QDomDocument>
#include <QTimer>
#include <QSettings>
#include <QMutexLocker>
#include <QMutex>
#include <QTime>
#include <stdexcept>
#include <qjson/parser.h>

#include "htrack.h"
#include "halbum.h"
#include "hartist.h"
#include "htoolbar.h"
#include "hobject.h"

QMutex rdioInterface;

QString noAccents(QString r) {
    r.replace(224,'a');
    r.replace(225,'a');
    r.replace(226,'a');
    r.replace(227,'a');
    r.replace(228,'a');
    r.replace(232,'e');
    r.replace(233,'e');
    r.replace(234,'e');
    r.replace(235,'e');
    r.replace(236,'i');
    r.replace(237,'i');
    r.replace(238,'i');
    r.replace(241,'i');
    r.replace(239,'i');
    r.replace(244,'o');
    r.replace(219,'u');
    r.replace(249,'u');
    r.replace(251,'u');
    r.replace(252,'u');
    r.replace(221,'y');
    r.replace(255,'y');
    r.replace('’','\'');
    r.replace('\' ','\'');
    while(r.endsWith(' ')) {
        r.chop(1);
    }
    return r;
}

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
    s_state(Stopped), s_shuffle(0), s_username(username), s_password(password), s_browser(), s_auth(new HAuthAction(s_browser,username,password)), s_lastTime_requeue(), s_ready(0)
{
    connect(s_auth,SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)),this,SLOT(oauth(QByteArray,QByteArray,QByteArray,QByteArray)));

    QEventLoop loop;
    loop.connect(s_auth, SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)), SLOT(quit()) );
    loop.connect(s_auth, SIGNAL(error(QString)), SLOT(quit()) );
    loop.exec();

    connect(HToolbar::singleton(),SIGNAL(shuffleToggled(bool)),this,SLOT(setShuffle(bool)));
}

HRdioInterface::HRdioInterface(QString rdioToken, QString rdioSecret, QString oauthToken, QString oauthSecret) :
    s_state(Stopped), s_shuffle(0), s_rdioToken(rdioToken.toUtf8()), s_rdioSecret(rdioSecret.toUtf8()), s_oauthToken(oauthToken.toUtf8()), s_oauthSecret(oauthSecret.toUtf8()),
    s_browser(), s_auth(0), s_lastTime_requeue(), s_ready(0)
{
    oauth(s_rdioToken,s_rdioSecret,s_oauthToken,s_oauthSecret);
    connect(HToolbar::singleton(),SIGNAL(shuffleToggled(bool)),this,SLOT(setShuffle(bool)));
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
    if(s_state==Stopped) {
        requeue();
    }
    if(a) tryPlay();
    else pause();
}

void HRdioInterface::pause() {
    s_browser.doJS("$('#api').rdio().pause()");
}

void HRdioInterface::next() {
    requeue(1);
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
    s_shuffle=a;
//    s_browser.doJS("$('#api').rdio().setShuffle("+QString(a?"true":"false")+")");
}

void HRdioInterface::queue(HArtist& artist) {
    Q_ASSERT(0);    //depricate me
    s_queue.push_back(&artist);
    if(s_state==Stopped) {
        requeue();
    }
//    QString tokenR=search(artist.getName(),"Artists","",artist.getName());
//    if(!tokenR.size()) return;
//    s_browser.doJS("$('#api').rdio().embed.rdio_queue('"+tokenR+"')");
//    setupPlayback();
}

void HRdioInterface::queue(HAlbum& album) {
    for(int i=0;i<album.getTracks().size();i++) {
        queue(*album.getTracks()[i]);
    }
}

void HRdioInterface::queue(HTrack& track) {
    s_queue.push_back(&track);
    track.getRdioKey();
    if(s_state==Stopped) {
        requeue();
    }
//    QString tokenR=search(track.getArtistName()+" "+track.getTrackName(),"Tracks","",track.getArtistName());
//    if(!tokenR.size()) return;
//    s_browser.doJS("$('#api').rdio().embed.rdio_queue('"+tokenR+"')");
//    setupPlayback();

//    if(s_state==Stopped) {
//        QEventLoop loop;
//        loop.connect( this, SIGNAL(playingTrackChanged(HTrack&)), SLOT(quit()) );
//        loop.connect( this, SIGNAL(positionChanged(double)), SLOT(quit()) );
//        loop.exec();
//    }
}

void HRdioInterface::play(HArtist& artist,bool clear) {
    Q_ASSERT(0);    //depricate me
    if(clear) {
        s_queue.clear();
        s_queue.push_back(&artist);
        if(s_state==Stopped) {
            requeue();
        }
        return;
    }
    QString tokenR=search(artist.getName(),"Artists","",artist.getName(),"");
    if(!tokenR.size()) return;
    s_browser.doJS("$('#api').rdio().play('"+tokenR+"')");
}

void HRdioInterface::play(HAlbum& album,bool clear) {
    Q_ASSERT(0);    //depricate me
    for(int i=0;i<album.getTracks().size();i++) {
        if(!i) play(*album.getTracks()[0],clear);
        else queue(*album.getTracks()[i]);
    }
}

void HRdioInterface::play(HTrack& track,bool clear) {
    if(clear) {
        s_queue.clear();
        s_queue.push_back(&track);
        if(s_state==Stopped) {
            requeue();
        }
        return;
    }
    QString tokenR=track.getRdioKey();
    if(!tokenR.size()||tokenR=="_NO_RESULT_") {
        requeue();
    }
    s_browser.doJS("$('#api').rdio().play('"+tokenR+"')");

//    if(s_state==Stopped) {
//        QEventLoop loop;
//        loop.connect( this, SIGNAL(playingTrackChanged(HTrack&)), SLOT(quit()) );
//        loop.exec();
//    }
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

void HRdioInterface::requeue(bool force) {
    if(!force) {
        if(s_state!=Stopped) return;
        if(s_lastTime_requeue.msecsTo(QTime::currentTime())<5678) {
            QTimer::singleShot(1234,this,SLOT(requeue()));
            return;
        }
    }
    s_lastTime_requeue=QTime::currentTime();
    if(s_queue.size()) {
        qDebug()<<s_shuffle;
        int track=s_shuffle?(qrand()%s_queue.size()):0;
        if(dynamic_cast<HTrack*>(s_queue.at(track))) {
            play(*dynamic_cast<HTrack*>(s_queue.takeAt(track)),0);
            return;
        }
        else if(dynamic_cast<HAlbum*>(s_queue.first())) {
            Q_ASSERT(0);    //depricate
            play(*dynamic_cast<HAlbum*>(s_queue.takeFirst()),0);
            return;
        }
        else if(dynamic_cast<HArtist*>(s_queue.first())) {
            Q_ASSERT(0);    //depricate
            play(*dynamic_cast<HArtist*>(s_queue.takeFirst()),0);
            return;
        }
    }
    QTimer::singleShot(5678,this,SLOT(requeue()));
}

void HRdioInterface::jsCallback(QString cb) {
    if(cb=="ready()") {
        s_ready=1;
        qDebug()<<"Ready to go!";

        return;
    }
    if(cb.startsWith("rdio.playingTrackChanged(")) {
        cb.remove("rdio.playingTrackChanged(");
        if(cb.size()<3) {
            qDebug()<<"Problem parsing rdio.playingTrackChanged(...)";
            return;
        }
        cb.chop(3);
        QStringList v=cb.split(',');
        QMap<QString,QVariant> result;
        for(int i=0;i<v.size();i++) {
            QStringList a=v[i].split(':');
            if(a.size()<2) continue;
            a[0].remove(0,1);
            a[0].chop(1);
            if(a[1].endsWith("\"")) {
                a[1].chop(1);
                a[1].remove(0,1);
                result.insert(a[0],a[1]);
            } else if(a[1].endsWith("true")) {
                result.insert(a[0],"true");
            } else if(a[1].endsWith("false")) {
                result.insert(a[0],"false");
            } else if(a[1].contains('.')){
                result.insert(a[0],a[1].toDouble());
            } else {
                result.insert(a[0],a[1].toInt());
            }
        }
        if(result.contains("duration")) s_currentInfo.duration=result["duration"].toInt();
        if(result.contains("is_clean")) s_currentInfo.clean=(result["is_clean"].toString()=="true");
        if(result.contains("artist")) s_currentInfo.arist=result["artist"].toString();
        if(result.contains("album")) s_currentInfo.album=result["album"].toString();
        if(result.contains("is_explicit")) s_currentInfo.expl=(result["is_explicit"].toString()=="true");
        if(result.contains("name")) s_currentInfo.name=result["name"].toString();
        s_currentInfo.valid=true;
//        HToolbar::singleton()->setPlaybackStatus("<A href=\"more\">"+s_currentInfo.name+" by "+s_currentInfo.arist);
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
            requeue();
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
            emit playingTrackChanged(HTrack::get(s_currentInfo.arist,s_currentInfo.name));
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
    QString f="$('#api').rdio().embed.rdio_queue('"+search("Ke$ha","Artists","","Ke$ha","")+"')";
    qDebug()<<"About to run f:"<<f;
    s_browser.doJS(f);
    setupPlayback();
}

QString HRdioInterface::search(QString search,QString types,QString albumF,QString artistF,QString trackF, bool noPar) {  //no auth!
    search=noAccents(search);
    albumF=noAccents(albumF);
    artistF=noAccents(artistF);
    trackF=noAccents(trackF);

    QMultiMap<QByteArray,QByteArray> map1;
    map1.insert("method", "search");
    map1.insert("query", QUrl::toPercentEncoding(search));
    map1.insert("types", QUrl::toPercentEncoding(types));
    map1.insert("format", QUrl::toPercentEncoding("xml"));
    QMultiMap<QByteArray,QByteArray> p=HBrowser::request(RDIO_CONSUMER_KEY,RDIO_CONSUMER_SECRET,"http://api.rdio.com/1/",map1,s_oauthToken,s_oauthSecret);
    if(p.values().size()<1) {
        qDebug()<<"ERR! Invalid response from rdio api!";
        return "";
    }

    try {
        QDomDocument doc;
        QString x=QString::fromUtf8(p.values()[0].data());
        x.remove(0,x.indexOf(">")+1);
        doc.setContent( x );

        QDomElement element = doc.documentElement();

        bool ok=1;
        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    bool ret=1;
                    QString toRet="";
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if(k.attributes().namedItem("name").nodeValue()=="key"&&ok) {
                                QString strx=j.toText().data();
                                if(strx.size()&&strx[0]=='r') strx='t'+strx;
                                toRet=strx;
                            }
                            if(k.attributes().namedItem("name").nodeValue()=="canStream") {
                                ok=(j.toText().data()=="true");
                            }
                            if(!albumF.isEmpty()&&k.attributes().namedItem("name").nodeValue()=="album") {
                                QString c=noAccents(j.toText().data()).toLower();
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(!c.startsWith(albumF)&&!albumF.startsWith(c)) {
                                    qDebug()<<"Fail on album name";
                                    ret=0;
                                }
                            }
                            if(!artistF.isEmpty()&&(k.attributes().namedItem("name").nodeValue()=="artist"||k.attributes().namedItem("name").nodeValue()=="albumArtist")) {
                                QString c=noAccents(j.toText().data()).toLower();
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(c!=artistF.toLower()) {
                                    qDebug()<<"Fail on artist name"<<c<<"VS"<<artistF.toLower()<<"FOR SEARCH"<<search;
                                    ret=0;
                                }
                            }
                            if(!trackF.isEmpty()&&(k.attributes().namedItem("name").nodeValue()=="name")) {
                                QString c=noAccents(j.toText().data()).toLower();
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(c.localeAwareCompare(trackF.toLower())) {
                                    qDebug()<<"Fail on track name"<<trackF<<"VS"<<noAccents(j.toText().data())<<"FOR SEARCH"<<search;
                                    ret=0;
                                }
                            }
                        }

                    }
                    if(ret&&toRet.size()) {
                        return toRet;
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    if(search.contains('(')||trackF.contains('(')||artistF.contains('(')) {
        if(search.contains('(')) search.truncate(search.indexOf('('));
        if(trackF.contains('(')) trackF.truncate(trackF.indexOf('('));
        if(artistF.contains('(')) artistF.truncate(artistF.indexOf('('));
        return HRdioInterface::search(search,types,albumF,artistF,trackF,1);
    } else {
        qWarning()<<"Could not find best search!";
        return "";
    }
}
