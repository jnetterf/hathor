#include "hrdioprovider.h"
#include "hauthaction.h"
#include <QEventLoop>
#include <QDebug>
#include <QDomDocument>
#include <QTimer>
#include <QSettings>
#include <QMutexLocker>
#include <QMutex>
#include <QTime>
#include <QtPlugin>
#include <stdexcept>
#include <qjson/parser.h>

#include "htrack.h"
#include "halbum.h"
#include "hartist.h"
#include "hobject.h"

#include <QTimer>
#include <QDesktopWidget>
#include <QCompleter>
#include <QGraphicsProxyWidget>
#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery>
#include <QGraphicsColorizeEffect>
#include <QDesktopServices>
#include <QPalette>
#include <QApplication>
#include "lastfmext.h"

HRdioLoginWidget::HRdioLoginWidget(HRdioProvider *rep, QWidget *parent): HRdioGraphicsView(parent),
  s_rep(rep)
{
    QSettings sett("Nettek","rdioKeys");
    if(sett.value("rdioEnabled").toString()=="FALSE") {
        QTimer::singleShot(0,this,SLOT(deleteLater()));
        return;
    }

    setMinimumHeight(768);
    setMinimumWidth(1000);
    if(qApp->desktop()->width()<1224) {
        setWindowState(Qt::WindowFullScreen);
    }
    tx=0;
    setBackgroundRole(QPalette::Base);
    sc = new QGraphicsScene;

    stage1=1;

    ///
    sc->setSceneRect(-10,-100,780,300);
    setScene(sc);
    tx = new QGraphicsTextItem;
    tx->setFont(QFont("Candara",60));
    if(sett.value("username").isValid()) tx->setPlainText("Welcome, "+sett.value("username").toString()+"!");
    else tx->setPlainText("");
    tx->show();

    sc->addItem(tx);
    tx->setPos(300,30);
    tx->setFont(QFont("Candara",25));
    qDebug()<<"About to restore...";
    rpx = new HRdioFadePixmap;
    rpx->setPixmap(QPixmap(":/icons/rdio.png").scaledToHeight(100,Qt::SmoothTransformation));
    rpx->setPos(000,0);
    rpx->show();
    sc->addItem(rpx);
    QTimer::singleShot(0,this,SLOT(finishLoading()));
}

void HRdioLoginWidget::finishLoading() {
//    tx->setFont(QFont("Candara",60));
    if(s_rep->restore()) {
        qDebug()<<"RESTORE!";
        QTimer::singleShot(0,this,SLOT(deleteLater()));
        return;
    }
    tx->setFont(QFont("Candara",60));

    tx->setPlainText("Press tab.");
    tx->setOpacity(0);
    ///

    QSettings settings("Nettek","lastfm_ext");
    s_superSecret_rdio=settings.value("keys_rdio").toMap();

    ///
    ranim=new QPropertyAnimation(rpx, "echoOpacity");
    ranim->setStartValue(1.0);
    ranim->setEndValue(0.0);
    ranim->setDuration(1000);
    ///
    ranim2=new QPropertyAnimation(tx, "opacity");
    ranim2->setStartValue(0.0);
    ranim2->setEndValue(1.0);
    ranim2->setDuration(1000);
    ///
    ranim3=0;
    ///
    ra=new MagicLineEdit;rb=new MagicLineEdit;
    ra->resize(400, 100);rb->resize(400,100);
    ra->setFont(QFont("Arial",60));rb->setFont(QFont("Arial",60));
    ra->setPlaceholderText("email"); rb->setPlaceholderText("password");
    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    connect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));
    rb->setEchoMode(QLineEdit::Password);
    sc->addWidget(ra)->setPos(380,0); sc->addWidget(rb)->setPos(800,0);
    ra->show();
    rb->show();
    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(onLoginChanged_rdio(QString)));

    affil=new QLabel;
    affil->setText("<a href=\"http://click.linksynergy.com/fs-bin/click?id=EtH0bqD6seI&offerid=221756.10000004&type=3&subid=0\" >"
                   "Sign up free!</a>");
    sc->addWidget(affil)->setPos(380,120);
    affil->show();
    affil->setPalette(QPalette(Qt::white));
    affil->setTextInteractionFlags(Qt::TextBrowserInteraction);
    affil->setFont(QFont("Candara",30));
    affil->adjustSize();
    connect(affil,SIGNAL(linkActivated(QString)),this,SLOT(openLink(QString)));

    nothanks=new QLabel;
    nothanks->setText("<a href=\"NO_THANKS\" >Skip!</a>");
    sc->addWidget(nothanks)->setPos(698,120);
    nothanks->show();
    nothanks->setPalette(QPalette(Qt::white));
    nothanks->setTextInteractionFlags(Qt::TextBrowserInteraction);
    nothanks->setFont(QFont("Candara",30));
    nothanks->adjustSize();
    connect(nothanks,SIGNAL(linkActivated(QString)),this,SLOT(skipRdio()));
    QTimer::singleShot(0,this,SLOT(rdio1()));
}

void HRdioLoginWidget::onLoginChanged_rdio(QString login) {
    if(!s_superSecret_rdio.value(login).isNull()) {
        rb->setText(s_superSecret_rdio.value(login).toString());
        return;
    }
    rb->setText("");
}

void HRdioLoginWidget::showTabHint_rdio() {
    if(!ra->text().size()) return;
    disconnect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));
    connect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));

    tx->show();
    ranim->start();
    QTimer::singleShot(300, ranim2, SLOT(start()));
    disconnect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    stage1=0;
    tx->setPos(0,0);
    tx->setFont(QFont("Candara",60));
    tx->setPlainText("Press tab.");


    QPropertyAnimation* pa_=new QPropertyAnimation(affil->graphicsProxyWidget(),"opacity");
    pa_->setStartValue(1);
    pa_->setEndValue(0);
    pa_->setDuration(500);
    pa_->start(QPropertyAnimation::DeleteWhenStopped);


}

void HRdioLoginWidget::doPassword_rdio() {

    disconnect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
//    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    disconnect(rb,SIGNAL(returnPressed()),this,SLOT(doLogin_rdio()));
    connect(rb,SIGNAL(returnPressed()),this,SLOT(doLogin_rdio()));

    ranim->setStartValue(0.0);
    ranim->setEndValue(1.0);
    ranim->setDuration(500);
    QTimer::singleShot(300, ranim, SLOT(start()));
    ranim2->setStartValue(1.0);
    ranim2->setEndValue(0.0);
    ranim2->setDuration(500);
    ranim2->start();
    disconnect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));

    if(!stage1){
        if(!ranim3)ranim3=new QPropertyAnimation(sc, "sceneRect");
        ranim3->setStartValue(sc->sceneRect());
        QRectF arect =sc->sceneRect();
        arect.translate(325,0);
        ranim3->setEndValue(arect);
        ranim3->setDuration(500);
        ranim3->start();
    }
    tx->setFont(QFont("Candara",50));
    tx->setPlainText("Press enter.");
    tx->setPos(800,120);
    tx->setOpacity(0);

    QPropertyAnimation* animA=new QPropertyAnimation(tx, "opacity");
    animA->setStartValue(0.0);
    animA->setEndValue(1.0);
    animA->setDuration(500);
    animA->start(QAbstractAnimation::DeleteWhenStopped);

    if(rb->text().size()) {
        QTimer::singleShot(700,this,SLOT(doLogin_rdio()));
    }
}

void HRdioLoginWidget::doLogin_rdio() {
    rb->setEnabled(0);
    tx->setPlainText("Loading...");
    if(!s_rep->login(ra->text(),rb->text())) {
        tx->setFont(QFont("Candara",60));
        tx->setPlainText("Press tab.");
        tx->setOpacity(0);
        tx->setPos(0,0);
        ranim->setStartValue(1.0);
        ranim->setEndValue(0.0);
        ranim->setDuration(500);
        ranim2->setStartValue(0.0);
        ranim2->setEndValue(1.0);
        ranim2->setDuration(500);

        rb->setEnabled(1);
        if(!ranim3) ranim3=new QPropertyAnimation(sc, "sceneRect");
        ranim3->setStartValue(sc->sceneRect());
        QRectF arect = sc->sceneRect();
        arect.translate(-325,0);
        ranim3->setEndValue(arect);
        ranim3->setDuration(400);
        ranim3->start();
        rb->setEnabled(1);
        ra->setText("");
        ra->setFocus();
        return;
    }
    {
        QPropertyAnimation* pa=new QPropertyAnimation(ra->graphicsProxyWidget(),"opacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    {
        QPropertyAnimation* pa=new QPropertyAnimation(rb->graphicsProxyWidget(),"opacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    {
        QPropertyAnimation* pa=new QPropertyAnimation(rpx,"echoOpacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    {
        QPropertyAnimation* pa=new QPropertyAnimation(tx,"opacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QTimer::singleShot(215,this,SLOT(rdio2()));
}

void HRdioLoginWidget::rdio1() {
    stage1=0;
    tx->show();
    setFocus();
    QTimer::singleShot(400,ra,SLOT(setFocus()));
//    ra->setFocus();
    affil->show();
    nothanks->show();
}

void HRdioLoginWidget::rdio2(int ax) {
    show();
    QRectF arect = sc->sceneRect();
    arect.translate(ax,800);
    sc->setSceneRect(arect);

    QSettings auth("Nettek","auth");
    auth.setValue("lfm.username",lastfm::ws::Username);
    auth.setValue("lfm.key",lastfm::ws::SessionKey);

    hide();
    emit showMainContext();
    deleteLater();
}

void HRdioLoginWidget::openLink(QString s) {
    QDesktopServices::openUrl(QUrl(s));
}

void HRdioLoginWidget::skipRdio() {
    QSettings sett("Nettek","rdioKeys");
    sett.setValue("rdioEnabled","FALSE");
    rdio2(0);
}






































void HRdioTrackInterface::play() {
    if(s_dontPlay) return;

    HRdioProvider::singleton()->setTI(this);

    if(s_playedYet) {
        HRdioProvider::singleton()->resume_lowlevel();
    } else {
        HRdioProvider::singleton()->play_lowlevel(s_key);
    }
}

void HRdioTrackInterface::pause() {
    HRdioProvider::singleton()->pause_lowlevel();
}

void HRdioTrackInterface::skip() {
    HRdioProvider::singleton()->disableTI();
    s_dontPlay=1;
    HRdioProvider::singleton()->pause_lowlevel();
    emit finished();
}

HAbstractTrackInterface::State HRdioTrackInterface::getState() const {
    HRdioProvider::State s = HRdioProvider::singleton()->s_state;
    if(s==HRdioProvider::Playing) return HAbstractTrackInterface::Playing;
    if(s==HRdioProvider::Stopped) return HAbstractTrackInterface::Stopped;
    if(s==HRdioProvider::Buffering) return HAbstractTrackInterface::Buffering;
    if(s==HRdioProvider::Paused) return HAbstractTrackInterface::Paused;
    if(s==HRdioProvider::PausedB) return HAbstractTrackInterface::Paused;
    qDebug()<<"INVALID STATE!!!";
    return HAbstractTrackInterface::Stopped;
}

QMutex rdioInterface;

QString rdio_standardized(QString r) {
    r=r.toLower();
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
    r.replace("’","\'");
    r.replace("\' ","\'");
    while(r.endsWith(' ')) {
        r.chop(1);
    }
    return r;
}

HRdioProvider* HRdioProvider::_singleton=0;

bool HRdioProvider::login(QString username, QString password) {
    ti=0;
    s_state=Stopped;
    s_username=username;
    s_password=password;
    s_auth=new HAuthAction(s_browser,username,password);
    s_ready=0;
    connect(s_auth,SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)),this,SLOT(oauth(QByteArray,QByteArray,QByteArray,QByteArray)));

    QEventLoop loop;
    loop.connect(s_auth, SIGNAL(gotOauth(QByteArray,QByteArray,QByteArray,QByteArray)), SLOT(quit()) );
    loop.connect(s_auth, SIGNAL(error(QString)), SLOT(quit()) );
    loop.connect(&s_browser,SIGNAL(rdioFail()),SLOT(quit()));
    loop.exec();

    if(ok()) {
        HPlayer::singleton()->installProvider(this);
        QSettings auth("Nettek","auth");
        auth.setValue("rdio.token",s_rdioToken);
        auth.setValue("rdio.secret",s_rdioSecret);
        auth.setValue("rdio.oauthToken",s_oauthToken);
        auth.setValue("rdio.oauthSecret",s_oauthSecret);

        return (_singleton=this);
    }
    else {
        qDebug()<<"Should delete maybe?";
        return 0;
    }
}

bool HRdioProvider::restore() {
    QSettings auth("Nettek","auth");
//    HRdioProvider* hri=new HRdioProvider(auth.value("rdio.token").toString(),
//                                           auth.value("rdio.secret").toString(),
//                                           auth.value("rdio.oauthToken").toString(),
//                                           auth.value("rdio.oauthSecret").toString());
    ti=0;
    s_state=Stopped;
    s_rdioToken=auth.value("rdio.token").toString().toUtf8();
    s_rdioSecret=auth.value("rdio.secret").toString().toUtf8();
    s_oauthToken=auth.value("rdio.oauthToken").toString().toUtf8();
    s_oauthSecret=auth.value("rdio.oauthSecret").toString().toUtf8();
    s_auth=0;
    s_ready=0;
    QSettings sett("Nettek","rdioKeys");
    if(sett.value("password").toString().size()) {
        HRdioLoginAction la(s_browser,sett.value("username").toString(),sett.value("password").toString());
        QEventLoop el;
        connect(&la,SIGNAL(done()),&el,SLOT(quit()));
        connect(&la,SIGNAL(error(QString)),&el,SLOT(quit()));
        la.init();
        el.exec();
        if(la.failed()) {
//            delete this;
            return 0;
        }
    }

    oauth(s_rdioToken,s_rdioSecret,s_oauthToken,s_oauthSecret);
    if(ok()) {
        HPlayer::singleton()->installProvider(this);
        return (_singleton=this);
    } else {
//        delete this;
        return 0;
    }
}

HRdioProvider::HRdioProvider() :
    ti(0), s_login(0), s_state(Stopped), s_browser(), s_auth(0), s_ready(0), s_calmDown(0)
{
    qDebug()<<"HRDIO";
    s_login=new HRdioLoginWidget(this);
}

bool HRdioProvider::ok() {
    return s_rdioToken.size()&&s_rdioSecret.size()&&s_oauthToken.size()&&s_oauthSecret.size();
}

void HRdioProvider::oauth(QByteArray rdioToken, QByteArray rdioSecret, QByteArray oauthToken,QByteArray oauthSecret) {
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
    s_browser.loadPage("http://shdw.x10.mx/cgi-bin/rgen.perl?"+QUrl::toPercentEncoding(s_playbackKey));
    connect(s_browser.s_webView->page(),SIGNAL(alert(const QString&)),this,SLOT(jsCallback(QString)));
}

void HRdioProvider::resume_lowlevel() {
    s_browser.doJS("$('#api').rdio().play()");
}

void HRdioProvider::pause_lowlevel() {
    s_browser.doJS("$('#api').rdio().pause()");
}

void HRdioProvider::seek_lowlevel(int sec) {
    s_browser.doJS("$('#api').rdio().seek("+QString::number(sec)+")");
}

void HRdioProvider::play_lowlevel(QString key) {
    s_browser.doJS("$('#api').rdio().play('"+key+"')");
}

void HRdioProvider::jsCallback(QString cb) {
    if(!ti) pause_lowlevel();
    if(cb.startsWith("ready(")) {
        if((cb.contains("\"isSubscriber\":false")&&cb.contains("\"freeRemaining\":0"))||cb.contains("null")) {
            qDebug()<<"Not a subscriber - so, no music!";
        } else {
            s_ready=1;
            qDebug()<<"Ready to go!";
        }
        qDebug()<<cb;
    }
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
        return;
    }
    if(cb.startsWith("rdio.playStateChanged(")) {
        cb.remove("rdio.playStateChanged(");
        cb.chop(1);
        s_state=(HRdioProvider::State)cb.toInt();
        HRdioTrackInterface* ti=HPlayer::singleton()->currentTrackInterface()?dynamic_cast<HRdioTrackInterface*>(HPlayer::singleton()->currentTrackInterface()):0;
        if(ti) ti->emitStateChanged();
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

QString HRdioProvider::search(QString search,QString types,QString albumF,QString artistF,QString trackF, bool noPar) {  //no auth!
    search=rdio_standardized(search);
    albumF=rdio_standardized(albumF);
    artistF=rdio_standardized(artistF);
    trackF=rdio_standardized(trackF);

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
                                QString c=rdio_standardized(j.toText().data());
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(!c.startsWith(albumF)&&!albumF.startsWith(c)) {
                                    qDebug()<<"Fail on album name";
                                    ret=0;
                                }
                            }
                            if(!artistF.isEmpty()&&(k.attributes().namedItem("name").nodeValue()=="artist"||k.attributes().namedItem("name").nodeValue()=="albumArtist")) {
                                QString c=rdio_standardized(j.toText().data());
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(c!=artistF) {
                                    qDebug()<<"Fail on artist name"<<c<<"VS"<<artistF<<"FOR SEARCH"<<search;
                                    ret=0;
                                }
                            }
                            if(!trackF.isEmpty()&&(k.attributes().namedItem("name").nodeValue()=="name")) {
                                QString c=rdio_standardized(j.toText().data());
                                if(noPar&&c.contains('(')) c.truncate(c.indexOf('('));
                                if(c.localeAwareCompare(trackF)) {
                                    qDebug()<<"Fail on track name"<<trackF<<"VS"<<rdio_standardized(j.toText().data())<<"FOR SEARCH"<<search;
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
        return HRdioProvider::search(search,types,albumF,artistF,trackF,1);
    } else {
        qWarning()<<"Could not find best search!";
        return "";
    }
}


QString HRdioProvider::getKey(HTrack& track) {
    for(int i=0;i<s_keys.size();i++) {
        if(&s_keys[i]->track==&track) {
            return s_keys[i]->getKey();
        }
    }
    s_keys.push_back(new Key(track));
    return s_keys.back()->getKey();
}

QString HRdioProvider::Key::getKey() {
    if(s_rdioKey.size()) return s_rdioKey;
    if(s_rdioKey_getting) {
        QEventLoop loop; connect(s_rdioKey_getting,SIGNAL(notify()),&loop,SLOT(quit())); loop.exec();
        return s_rdioKey;
    }
//    QSettings sett("Nettek","rdioKeys");
//    if(sett.value("key for "+track.getArtistName()+"__"+track.getTrackName()).isValid()) {
//        return s_rdioKey=sett.value("key for "+track.getArtistName()+"__"+track.getTrackName()).toString();
//    }
    s_rdioKey_getting=new HRunOnceNotifier;
    s_rdioKey=HRdioProvider::singleton()->search(track.getArtistName()+" "+track.getTrackName(),"Tracks","",track.getArtistName(),track.getTrackName());
    if(!s_rdioKey.size()) s_rdioKey="_NO_RESULT_";
    s_rdioKey_getting->emitNotify();
    s_rdioKey_getting=0;
//    sett.setValue("key for "+track.getArtistName()+"__"+track.getTrackName(),s_rdioKey);
    return s_rdioKey;
}

QList<HSendScoreTriplet_Rdio*> HSendScoreTriplet_Rdio::s_list;

void HSendScoreTriplet_Rdio::doMagic() {
    if(s_calmDown) {
        qWarning()<<"We should never get here!!!";
        return;
    }
    s_calmDown=1;
    if(!s_list.size()) s_list.push_back(0);
    bool ok=HRdioProvider::singleton()->getKey(track)!="_NO_RESULT_";
    QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(int,ok?(s_ready?80:1):0),Q_ARG(HAbstractTrackProvider*,HRdioProvider::singleton()));
    while(s_list.size()) {
        if(s_list.first()==0) s_list.pop_front();
        else {
            QTimer::singleShot(0,s_list.takeFirst(),SLOT(doMagic()));
            break;
        }
//        QTimer::singleShot(0,this,SLOT(doMagic()));
    }
    s_calmDown=0;
}

Q_EXPORT_PLUGIN2(hrdio_provider, HRdioProvider)
