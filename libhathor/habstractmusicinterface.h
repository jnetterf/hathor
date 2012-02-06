#ifndef HABSTRACTMUSICINTERFACE_H
#define HABSTRACTMUSICINTERFACE_H

#include "libhathor_global.h"

#include "htrack.h"
#include "halbum.h"
#include "hartist.h"

#include <QTime>
#include <QLayout>

#include <phonon/MediaObject>

class LIBHATHORSHARED_EXPORT HAbstractTrackInterface : public QObject {
    Q_OBJECT
    friend class HPlayer_PotentialTrack;
public:
    enum State {
        Playing=0,
        Paused=1,
        Stopped=2,
        Buffering=3,
        Searching=4
    };
protected:
    friend class HPlayer;

    virtual void play()=0;
    virtual void pause()=0;     // _MUST_ be playing
    virtual void skip()=0;      // _MUST_ be playing
    virtual State getState() const = 0;
    HTrack& s_track;
public:
    HAbstractTrackInterface(HTrack& track) : s_track(track) {}
    HTrack& getTrack() const { return s_track; }

signals:
    void finished();
    void stateChanged(HAbstractTrackInterface::State s);
};

class LIBHATHORSHARED_EXPORT HAbstractTrackProvider {
    friend class HPlayer;
    friend class HPlayer_PotentialTrack;
    virtual int globalScore() = 0;  //In general (from 0 to 100), how good is this provider? Local is 100, a provider which takes 22 minutes to respond is 0.
    virtual void sendScore(HTrack& track,QObject* obj,QString slot)=0;
        //score goes from 100=DEFINATE match to 0=NO match - Rdio should be 100 and Youtube should be 20. 0 means it cannot be played
        //When the score is determined, send it along with a pointer to this to obj->slot using QMetaObject::invokeMethod(..., Q_ARG(int,...), Q_ARG(HAbstractTrackProvider*,...)
    virtual void sendTrack(HTrack& track,QObject* obj,QString slot)=0;
        //When the HAbstractTrackInterface* is created, send it and this to obj->slot using
        //QMetaObject::invokeMethod(..., QARG(HAbstractTrackInterface*,...) Q_ARG(HAbstractTrackProvider*,...)
        // MUST RETURN IMMEDIATELY!

    virtual QWidget* initWidget() = 0;
        // When this provider is initilized, initWidget() is shown until it is destroyed. Use this if you need login info,
        // or you take a long time to start. If not needed, just return 0.

    virtual QString name() = 0;
};

Q_DECLARE_INTERFACE(HAbstractTrackProvider, "com.Nettek.Hathor.AbstractTrackProvider/1.01")

class LIBHATHORSHARED_EXPORT HPlayer_PotentialTrack : public QObject {
    Q_OBJECT
    friend class HPlayer;
    friend class HStandardQueue;
    int s_score;
    HAbstractTrackInterface* s_interface;
    HAbstractTrackProvider* s_bestProviderSoFar;
    HTrack& track;
    QMutex mutex;
    int s_rem;
    bool s_readyToPlay, s_readyToSkip, s_readyToPause;
public:
    HAbstractTrackInterface* p_ti;
    HPlayer_PotentialTrack(HTrack& t) : s_score(0), s_interface(0),s_bestProviderSoFar(0), track(t), s_rem(0), s_readyToPlay(0), s_readyToSkip(0), s_readyToPause(0), p_ti(0) {}
    HAbstractTrackProvider* getProvider();
signals:
    void startedPlaying(HTrack& t);
    void finished();
    void cantFind();
    void stateChanged(HAbstractTrackInterface::State);
public slots:
    void regProvider(HAbstractTrackProvider* tp);
    void regScore(int score,HAbstractTrackProvider* tp);
    void regAB(HAbstractTrackInterface* ti,HAbstractTrackProvider* tp);
    void play();
    void resume();
    void skip();
    void pause();
};

class LIBHATHORSHARED_EXPORT HAbstractQueue : public QObject {
    Q_OBJECT
public slots:
    virtual void queue(QList<HTrack*> tracks) {
        foreach(HTrack* t,tracks) queue(t);
    }
    virtual void queue(HTrack* track)=0;
    virtual void detach()=0;
    virtual void skip()=0;
    virtual void stop()=0;
    virtual void pause()=0;
    virtual void resume()=0;
    virtual void playNext()=0;
public:
    virtual HAbstractTrackInterface* currentTrackInterface() { return 0; }
    virtual HTrack* currentTrack()=0;
signals:
    void stateChanged(HAbstractTrackInterface::State);
    void trackChanged(HTrack& t);
    void shuffleToggled(bool);
    void searching();
};

class LIBHATHORSHARED_EXPORT HStandardQueue : public HAbstractQueue {
    Q_OBJECT
    QList<HAbstractTrackProvider*>& s_providers;
    bool& s_shuffle;
    HPlayer_PotentialTrack* s_currentTrack;
    QList<HPlayer_PotentialTrack*> s_queue;
    bool s_attached;
    QMutex s_lock;
public:
    HStandardQueue(QList<HAbstractTrackProvider*>& providers, bool& shuffle) : s_providers(providers), s_shuffle(shuffle), s_currentTrack(0), s_attached(1) {}
    HTrack* currentTrack() { if(s_currentTrack) return &s_currentTrack->track; else return 0;}
    HAbstractTrackInterface* currentTrackInterface() { return s_currentTrack?s_currentTrack->p_ti:0; }
    HPlayer_PotentialTrack* currentPotentialTrack() { return s_currentTrack; }
signals:
    void cantFind();
public slots:
    void queue(HTrack* track);
    void queue(HAlbum* album);

    void detach() { stop(); s_attached=0; }

    void skip();
    void stop();
    void pause();
    void resume();

    void playNext();
};

class LIBHATHORSHARED_EXPORT HScrobbler : public QObject {
    Q_OBJECT
    static HScrobbler* s_singleton;
    QTime s_heuristic;
    HTrack* s_cur;
    HScrobbler() : s_heuristic(QTime::currentTime()), s_cur(0) {}
public:
    static HScrobbler* singleton() { if(!s_singleton) s_singleton = new HScrobbler; return s_singleton; }
public slots:
    void onSongStart(HTrack* t);
};

class LIBHATHORSHARED_EXPORT HPlayer : public QObject{
    Q_OBJECT
    QList<HAbstractTrackProvider*> s_providers;
    static HPlayer* s_singleton;
    bool s_shuffle;
    HAbstractQueue* Q;
    HPlayer() : s_providers(), s_shuffle(0), Q(0) { s_singleton=this; }
public:
    static HPlayer* singleton() { if(!s_singleton) {s_singleton=new HPlayer;} return s_singleton; }
    HTrack* currentTrack() { return Q?Q->currentTrack():0; }
    HAbstractTrackInterface* currentTrackInterface() { return Q?Q->currentTrackInterface():0; }
    HStandardQueue* getStandardQueue();
    QString getProviderName();

public slots:
    void installProvider(HAbstractTrackProvider*);
    void clear();
    void skip();
    void stop();
    void pause();
    void resume();
    void setShuffle(bool a) { emit shuffleToggled(s_shuffle=a); }

    void playNext();

    void loadPlugins(QLayout* l);

signals:
    void stateChanged(HAbstractTrackInterface::State);
    void trackChanged(HTrack& t);
    void shuffleToggled(bool);
    void cantFind();
};

class LIBHATHORSHARED_EXPORT HPhononTrackInterface : public HAbstractTrackInterface {
    Q_OBJECT
    bool s_played;
    bool s_stopped;
    Phonon::MediaObject* s_mo;
    void play() { if(s_mo&&!s_stopped) s_mo->play(); }
    void pause() { if(s_mo&&!s_stopped) s_mo->pause(); }
    void skip() { if(s_mo&&!s_stopped) s_mo->stop(); disconnect(s_mo,0,this,0); emit finished(); s_stopped=1; }
    HAbstractTrackInterface::State getState() const;
public:
    HPhononTrackInterface(HTrack& track,QString url) : HAbstractTrackInterface(track), s_played(0), s_stopped(0), s_mo(Phonon::createPlayer(Phonon::MusicCategory,Phonon::MediaSource(url))) {
        if(s_mo) connect(s_mo,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SLOT(onStateChanged()));
    }
public slots:
    void onStateChanged() {
        if(s_mo->state()==Phonon::ErrorState) emit finished();
        if(s_mo->state()==Phonon::PlayingState) s_played=1;
        if(s_mo->state()==Phonon::StoppedState&&s_played) emit finished();
        emit stateChanged(getState());
    }
};


#endif // HABSTRACTMUSICINTERFACE_H
