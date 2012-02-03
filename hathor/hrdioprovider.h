#ifndef HRDIOINTERFACE_H
#define HRDIOINTERFACE_H

#include "hloginwidget.h"
#include <QString>
#include "hbrowser.h"
#include "hauthaction.h"
#include "hobject.h"
#include <QObject>
#include "habstractmusicinterface.h"
#include <QTime>
#include <QNetworkCookieJar>

class HArtist;
class HAlbum;
class HTrack;

class HRdioProvider;

class HRdioLoginWidget : public HGraphicsView {
    Q_OBJECT
    HRdioProvider* s_rep;
public:
    explicit HRdioLoginWidget(HRdioProvider* rep, QWidget *parent = 0);

public slots:
    // LOGIN
    void rdio1();
    void showTabHint_rdio();
    void doPassword_rdio();
    void doLogin_rdio();
    void onLoginChanged_rdio(QString login);

    void rdio2(int a=0);

    void openLink(QString);
    void skipRdio();
    void finishLoading();

signals:
    void showMainContext();

private:
    bool stage1;
    FadePixmap* rpx;
    QGraphicsScene* sc;
    QGraphicsTextItem* tx;
    QLabel* affil, * nothanks;
    QPropertyAnimation* ranim,*ranim2,*ranim3;
    MagicLineEdit*ra,*rb;
    QMap<QString,QVariant> s_superSecret_rdio;
};

class HNetworkCookieJar: public QNetworkCookieJar {
    Q_OBJECT
public:
    HNetworkCookieJar(QObject *parent = 0) : QNetworkCookieJar(parent) {}
    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) {
        emit newCookie();
        return QNetworkCookieJar::setCookiesFromUrl(cookieList,url);
    }

signals:
    void newCookie();
};

class HRdioLoginAction : public HAction {
public:
    Q_OBJECT
    QString s_un, s_psswd;
    bool s_failed;
public:
    HRdioLoginAction(HBrowser& broser,QString un,QString psswd) : HAction(broser), s_un(un), s_psswd(psswd), s_failed(0) {}
    bool failed() { return s_failed; }
public slots:
    void init() {
        s_browser.loadPage("http://www.rdio.com/secure/login/?cn=secure_login_result&popup=1");
        connect(&s_browser,SIGNAL(ready()),this,SLOT(next_1()));
        QTimer::singleShot(9000,this,SLOT(fail()));
//        s_browser.show();
    }
    void next_1() {
        if(s_failed) return;
        s_browser.setInput("id_email",s_un);
        s_browser.setInput("id_password",s_psswd);
        s_browser.doJS("document.getElementById(\"signinButton\").click()");
        disconnect(&s_browser,SIGNAL(ready()),this,SLOT(next_1()));
        connect(&s_browser,SIGNAL(ready()),this,SIGNAL(done()));
        QTimer::singleShot(7400,this,SLOT(fail()));
        HNetworkCookieJar* ncj=dynamic_cast<HNetworkCookieJar*>(s_browser.s_webView->page()->networkAccessManager()->cookieJar());
        if(!ncj) s_browser.s_webView->page()->networkAccessManager()->setCookieJar(ncj=new HNetworkCookieJar);
        connect(ncj,SIGNAL(newCookie()),this,SIGNAL(done()));
        //Invalid username or password, please try again.
    }
    void fail() {
        s_failed=1;
        emit error("Could not load");
    }
};

class HRdioTrackInterface : public HAbstractTrackInterface {
    QString s_key;
    bool s_playedYet;
    bool s_dontPlay;

    void play();
    void pause();
    void skip();
    State getState() const;
// void finished(); void stateChanged(State s);
public:
    HRdioTrackInterface(HTrack& track, QString key) : HAbstractTrackInterface(track), s_key(key), s_playedYet(0), s_dontPlay(0) {}
    void emitStateChanged() { qDebug()<<"EMITSTATECHANGED"; emit stateChanged(getState()); if(getState()==Playing) s_playedYet=1; if(getState()==Stopped&&s_playedYet) emit finished(); }
};

struct HSendScoreTriplet_Rdio {
    HTrack& track;
    QObject* o;
    QString m;
    bool& s_ready;
    static QList<HSendScoreTriplet_Rdio*> s_list;
public:
    HSendScoreTriplet_Rdio(HTrack& a,QObject* b, QString c, bool&r) : track(a), o(b), m(c), s_ready(r) {
        if(!s_list.size()) doMagic();
        else s_list.push_back(this);
    }

    void doMagic();

};

class HRdioProvider : public QObject, public HAbstractTrackProvider {
    Q_OBJECT
    Q_INTERFACES(HAbstractTrackProvider)
    friend class HTrack;
    HRdioTrackInterface* ti;
    HRdioLoginWidget* s_login;

public: //HAbstractTrackProvider
    int globalScore() { return 90; /*Rdio is really good!*/ }
    void sendScore(HTrack& track,QObject* o,QString m) {
        new HSendScoreTriplet_Rdio(track,o,m,s_ready);
    }
    void sendTrack(HTrack& track,QObject* o,QString m) {
        HAbstractTrackInterface* ti=new HRdioTrackInterface(track,getKey(track));
        QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HAbstractTrackInterface*,ti),Q_ARG(HAbstractTrackProvider*,this));
    }
    QWidget* initWidget() { return s_login; }

public slots: // Playback and queue:
    void seek_lowlevel(int sec);
    void pause_lowlevel();
    void play_lowlevel(QString key);
    void resume_lowlevel();

    void setTI(HRdioTrackInterface* cti) { ti=cti; }
    void disableTI() { ti=0; }

private:
    struct CurrentTrackInfo {
        int duration;
        QString arist;
        QString name;
        QString album;
        bool clean;
        bool expl;
        bool valid;
        CurrentTrackInfo() : valid(0) {}
    } s_currentInfo;

    double s_curPos;

public:
    enum State {
        Paused=0,
        Playing=1,
        Stopped=2,
        Buffering=3,
        PausedB=4   //brought to you by the department of redundency department
    } s_state;

    struct Key {
        HTrack& track;
        QString s_rdioKey;
        HRunOnceNotifier* s_rdioKey_getting;
        QString getKey();
        Key(HTrack& ctrack) : track(ctrack), s_rdioKey_getting(0) {}
    };

    QList< Key* > s_keys;
    QString getKey(HTrack& track);

private:
    QString s_username, s_password;
    QByteArray s_rdioToken,s_rdioSecret, s_oauthToken,s_oauthSecret;
    QString s_playbackKey;

    HBrowser s_browser;
    HAuthAction* s_auth;

    bool s_ready;
    static HRdioProvider* _singleton;

//    HRdioProvider(QString username, QString password);
//    HRdioProvider(QString rdioToken, QString rdioSecret, QString oauthToken, QString oauthSecret);
    State getState() { return s_state; }
public:
    HRdioProvider();
    bool login(QString username, QString password);
    bool restore();
    static HRdioProvider* singleton() { return _singleton; } //can return null
    bool ok();
    bool ready() { return s_ready; }

    QString search(QString search,QString types, QString albumF, QString artistF,QString trackF, bool noPar=0);   // gets best token

//    QString trackName() { return s_currentInfo.name; }
//    QString artistName() { return s_currentInfo.arist; }

public slots:
    void oauth(QByteArray rdioToken, QByteArray rdioSecret, QByteArray oauthToken,QByteArray oauthSecret);
    void jsCallback(QString cb);

signals:
    void positionChanged(double newPos);
};

#endif // HRDIOINTERFACE_H
