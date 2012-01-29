#ifndef HRDIOINTERFACE_H
#define HRDIOINTERFACE_H

#include <QString>
#include "hbrowser.h"
#include "hauthaction.h"
#include "hobject.h"
#include <QObject>
#include "habstractmusicinterface.h"
#include <QTime>

class HArtist;
class HAlbum;
class HTrack;

class HRdioLoginAction : public HAction {
public:
    Q_OBJECT
    QString s_un, s_psswd;
public:
    HRdioLoginAction(HBrowser& broser,QString un,QString psswd) : HAction(broser), s_un(un), s_psswd(psswd) {}
public slots:
    void init() {
        s_browser.loadPageHTTPS("https://www.rdio.com/secure/login/?cn=secure_login_result");
        connect(&s_browser,SIGNAL(ready()),this,SLOT(next_1()));
        QTimer::singleShot(7400,this,SIGNAL(done()));
//        s_browser.show();
    }
    void next_1() {
        s_browser.setInput("id_email",s_un);
        s_browser.setInput("id_password",s_psswd);
        s_browser.doJS("document.getElementById(\"signinButton\").click()");
        disconnect(&s_browser,SIGNAL(ready()),this,SLOT(next_1()));
        QTimer::singleShot(200,this,SIGNAL(done()));
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

public: //HAbstractTrackProvider
    int globalScore() { return 90; /*Rdio is really good!*/ }
    void sendScore(HTrack& track,QObject* o,QString m) {
        new HSendScoreTriplet_Rdio(track,o,m,s_ready);
    }
    void sendTrack(HTrack& track,QObject* o,QString m) {
        HAbstractTrackInterface* ti=new HRdioTrackInterface(track,getKey(track));
        QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HAbstractTrackInterface*,ti),Q_ARG(HAbstractTrackProvider*,this));
    }

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

    HRdioProvider(QString username, QString password);
    HRdioProvider(QString rdioToken, QString rdioSecret, QString oauthToken, QString oauthSecret);
    State getState() { return s_state; }
public:
    static HRdioProvider* login(QString username, QString password);
    static HRdioProvider* restore();
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
