#ifndef HABSTRACTMUSICINTERFACE_H
#define HABSTRACTMUSICINTERFACE_H

#include "libhathor_global.h"

#include "htrack.h"
#include "halbum.h"
#include "hartist.h"

#include <phonon/MediaObject>

class LIBHATHORSHARED_EXPORT HAbstractTrackInterface : public QObject {
    Q_OBJECT
    friend class HPlayer_PotentialTrack;
public:
    enum State {
        Playing=0,
        Paused=1,
        Stopped=2,
        Buffering=3
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
};

Q_DECLARE_INTERFACE(HAbstractTrackProvider, "com.Nettek.Hathor.AbstractTrackProvider/1.01")

class HPlayer_PotentialTrack : public QObject {
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
signals:
    void startedPlaying(HTrack& t);
    void finished();
    void stateChanged(HAbstractTrackInterface::State);
public slots:
    void regProvider(HAbstractTrackProvider* tp) {
        if(s_rem==-1) return;
        ++s_rem;
        tp->sendScore(track,this,"regScore");
    }

    void regScore(int score,HAbstractTrackProvider* tp) {
        if(s_rem==-1) return;
        QMutexLocker locker(&mutex);
        qDebug()<<"###GOT SCORE"<<score<<"VS"<<s_score<<"FROM"<<tp<<s_rem-1<<"REMAINING";
        if(score>s_score&&score) {
            if(p_ti) {
                delete p_ti;
                p_ti=0;
            }
            s_score=score;
            s_bestProviderSoFar=tp;
            tp->sendTrack(track,this,"regAB");
        } else {
            --s_rem;
            if(s_readyToSkip) skip();
            else if(s_readyToPlay) play();
        }
    }

    void regAB(HAbstractTrackInterface* ti,HAbstractTrackProvider* tp) {
        if(s_rem==-1) return;
        QMutexLocker locker(&mutex);
        --s_rem;
        if(tp!=s_bestProviderSoFar) {
            delete ti;
            return;
        }
        p_ti=ti;
        if(s_readyToSkip) skip();
        else if(s_readyToPlay) play();
    }

    void play() {
        if(s_rem>0) { s_readyToPlay=1; return; }
        if(s_readyToSkip) { skip(); return; }
        if(!p_ti) {
            emit finished();
            return;
        }
        s_rem=-1;
        connect(p_ti,SIGNAL(finished()),this,SIGNAL(finished()));
        connect(p_ti,SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SIGNAL(stateChanged(HAbstractTrackInterface::State)));
        if(!s_readyToPause) p_ti->play();
        qDebug()<<"PLAYING!!!"<<p_ti->getTrack().getTrackName();
        emit startedPlaying(p_ti->getTrack());
    }

    void resume() {
        if(p_ti&&s_rem==-1) { p_ti->play(); }
    }

    void skip() {
        qDebug()<<"SKIP!!!"<<s_readyToSkip<<p_ti<<s_rem;
        if(s_rem>0) { s_readyToSkip=1; }
        if(p_ti) { p_ti->skip(); }
    }

    void pause() {
        if(s_rem>0) { s_readyToPause=1; return; }
        else if(s_rem==-1&&p_ti) { p_ti->pause(); }
    }
};

class HAbstractQueue : public QObject {
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
    virtual HAbstractTrackInterface* currentTrackInterface() { return 0; }
public:
    virtual HTrack* currentTrack()=0;
signals:
    void stateChanged(HAbstractTrackInterface::State);
    void trackChanged(HTrack& t);
    void shuffleToggled(bool);
};

class HStandardQueue : public HAbstractQueue {
    Q_OBJECT
    QList<HAbstractTrackProvider*>& s_providers;
    bool& s_shuffle;
    HPlayer_PotentialTrack* s_currentTrack;
    QList<HPlayer_PotentialTrack*> s_queue;
    bool s_attached;
public:
    HStandardQueue(QList<HAbstractTrackProvider*>& providers, bool& shuffle) : s_providers(providers), s_shuffle(shuffle), s_currentTrack(0), s_attached(1) {}
    HTrack* currentTrack() { if(s_currentTrack) return &s_currentTrack->track; else return 0;}
    HAbstractTrackInterface* currentTrackInterface() { return s_currentTrack?s_currentTrack->p_ti:0; }
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

class LIBHATHORSHARED_EXPORT HPlayer : public QObject{
    Q_OBJECT
    QList<HAbstractTrackProvider*> s_providers;
    static HPlayer* s_singleton;
    bool s_shuffle;
    HAbstractQueue* Q;
    HPlayer() : s_providers(), s_shuffle(0), Q(0) { s_singleton=this; loadPlugins(); }
public:
    static HPlayer* singleton() { if(!s_singleton) {s_singleton=new HPlayer;} return s_singleton; }
    HTrack* currentTrack() { return Q?Q->currentTrack():0; }
    HAbstractTrackInterface* currentTrackInterface() { return Q?Q->currentTrackInterface():0; }
    HStandardQueue* getStandardQueue();

public slots:
    void installProvider(HAbstractTrackProvider*);
    void clear();
    void skip();
    void stop();
    void pause();
    void resume();
    void setShuffle(bool a) { emit shuffleToggled(s_shuffle=a); }

    void playNext();

    void loadPlugins();

signals:
    void stateChanged(HAbstractTrackInterface::State);
    void trackChanged(HTrack& t);
    void shuffleToggled(bool);
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
