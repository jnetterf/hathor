#include "habstractmusicinterface.h"
#include "hnettloger.h"
#include "lastfmext.h"
#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QPluginLoader>
#include <QNetworkReply>
#include <QWidget>
#include "hplugin.h"

HPlayer* HPlayer::s_singleton=0;
HScrobbler* HScrobbler::s_singleton;

void HPlayer_PotentialTrack::regProvider(HAbstractTrackProvider *tp) {
    if(s_rem==-1) return;
    ++s_rem;
    tp->sendScore(track,this,"regScore");
}

void HPlayer_PotentialTrack::regScore(int score, HAbstractTrackProvider *tp) {
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
        --s_rem;
    } else {
        --s_rem;
    }
    if(!s_rem) {
        if(s_bestProviderSoFar) s_bestProviderSoFar->sendTrack(track,this,"regAB");
        else {
            emit cantFind();
            emit finished();
        }
    }
}

HAbstractTrackProvider* HPlayer_PotentialTrack::getProvider() {
    return s_bestProviderSoFar;
}

void HPlayer_PotentialTrack::regAB(HAbstractTrackInterface *ti, HAbstractTrackProvider *tp) {
    if(s_rem==-1) {
        return;
    }
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    if(tp!=s_bestProviderSoFar) {
        delete ti;
        return;
    }
    p_ti=ti;
    if(s_readyToSkip) skip();
    else if(s_readyToPlay) play();
}

void HPlayer_PotentialTrack::play() {
    if(s_rem>0) { s_readyToPlay=1; return; }
    if(s_readyToSkip) { skip(); return; }
    if(!p_ti) {
        emit finished();
        return;
    }
    s_rem=-1;
    connect(p_ti,SIGNAL(finished()),this,SIGNAL(finished()));
    connect(p_ti,SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SIGNAL(stateChanged(HAbstractTrackInterface::State)));
    if(!s_readyToPause) {
        p_ti->play();
        emit startedPlaying(p_ti->getTrack());
    }
}

void HPlayer_PotentialTrack::resume() {
    if(p_ti&&s_rem==-1) { p_ti->play(); }
}

void HPlayer_PotentialTrack::skip()  {

//    qDebug()<<"SKIP!!!"<<s_readyToSkip<<p_ti<<s_rem;
    if(s_rem>0) { s_readyToSkip=1; }
    if(p_ti) { p_ti->skip(); }
}

void HPlayer_PotentialTrack::pause() {
    if(s_rem>0) { s_readyToPause=1; return; }
    else if(s_rem==-1&&p_ti) { p_ti->pause(); }
}


HAbstractTrackInterface::State HPhononTrackInterface::getState() const {
    if(s_mo) {
        switch(s_mo->state()) {
        case Phonon::LoadingState:
        case Phonon::BufferingState:
            return Buffering;
            break;
        case Phonon::StoppedState:
        case Phonon::ErrorState:
            return Stopped;
            break;
        case Phonon::PlayingState:
            return Playing;
            break;
        case Phonon::PausedState:
            return Paused;
            break;
        }
    }
    return Stopped;
}


void HPlayer::installProvider(HAbstractTrackProvider* p) {
    int score=p->globalScore();
    bool ok=0;
    for(int i=0;i<s_providers.size();i++) {
        if(score>=s_providers[i]->globalScore()) {
            s_providers.insert(i,p);
            ok=1;
            break;
        }
    }
    if(!ok) s_providers.push_back(p);
}

void HPlayer::clear() {
    if(Q) {
        Q->detach();
        Q=0;
    }
}

HStandardQueue* HPlayer::getStandardQueue() {
    HStandardQueue* sq=dynamic_cast<HStandardQueue*>(Q);
    if(sq) return sq;

    if(Q) clear();
    Q=sq=new HStandardQueue(s_providers,s_shuffle);
    connect(sq,SIGNAL(cantFind()),this,SIGNAL(cantFind()));
    connect(Q,SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SIGNAL(stateChanged(HAbstractTrackInterface::State)));
    connect(Q,SIGNAL(trackChanged(HTrack&)),this,SIGNAL(trackChanged(HTrack&)));
    connect(Q,SIGNAL(shuffleToggled(bool)),this,SIGNAL(shuffleToggled(bool)));
    return sq;
}

QString HPlayer::getProviderName()  {
    HStandardQueue* sq= dynamic_cast<HStandardQueue*>(Q);
    if(!sq) return "unknown source";
    else if(sq->currentPotentialTrack()&&sq->currentPotentialTrack()->getProvider()) return sq->currentPotentialTrack()->getProvider()->name();
    return "bad provider...";
}

void HStandardQueue::queue(HTrack *track) {
    if(!track) return;
    QMutexLocker l(&s_lock);
    if(!s_currentTrack) emit stateChanged(HAbstractTrackInterface::Searching);

    HPlayer_PotentialTrack* pt=new HPlayer_PotentialTrack(*track);
    s_queue.push_back(pt);
    connect(pt,SIGNAL(cantFind()),this,SIGNAL(cantFind()));
    for(int i=0;i<s_providers.size();i++) {
        pt->regProvider(s_providers[i]);
    }
    if(!s_currentTrack) playNext();
}

void HStandardQueue::queue(HAlbum* album) {
    if(!album) return;
    album->sendTracks(this,"queue");
}

void HStandardQueue::skip() {
    if(s_currentTrack) {
        s_currentTrack->skip();
        s_currentTrack=0;
    } else {
        playNext();
    }
}

void HPlayer::skip() {
    if(Q) Q->skip();
}

void HStandardQueue::stop() {
    s_queue.clear();
    if(s_currentTrack) {
        s_currentTrack->skip();
    }
}

void HPlayer::stop() {
    if(Q) Q->stop();
}

void HStandardQueue::pause() {
    if(s_currentTrack) {
        s_currentTrack->pause();
    }
}

void HPlayer::pause() {
    if(Q) Q->pause();
}

void HStandardQueue::resume() {
    if(s_currentTrack) {
        s_currentTrack->resume();
    } else playNext();
}

void HPlayer::resume() {
    if(Q) Q->resume();
}

void HStandardQueue::playNext() {

    if(s_currentTrack) {
        disconnect(s_currentTrack,0,this,0);
        s_currentTrack->skip();
        s_currentTrack=0;
    }

    if(s_queue.size()) {
        int i=s_shuffle?(qrand()%s_queue.size()):0;
        s_currentTrack=s_queue[i];
        s_queue.removeAt(i);
        connect(s_currentTrack,SIGNAL(finished()),this,SLOT(playNext()));
        connect(s_currentTrack,SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SIGNAL(stateChanged(HAbstractTrackInterface::State)));
        connect(s_currentTrack,SIGNAL(startedPlaying(HTrack&)),this,SIGNAL(trackChanged(HTrack&)));
        emit stateChanged(HAbstractTrackInterface::Searching);
        HScrobbler::singleton()->onSongStart(&s_currentTrack->track);
        s_currentTrack->play();
    } else {
        HScrobbler::singleton()->onSongStart(0);
        s_currentTrack=0;
        emit stateChanged(HAbstractTrackInterface::Stopped);
    }
}

void HPlayer::playNext() {
    if(Q) Q->playNext();
}

void HPlayer::loadPlugins(QLayout *l) {
    qDebug()<<qApp->applicationDirPath();
    cl.push_back(qApp->applicationDirPath());
    cl.push_back("/usr/share/hathor-20120128");
    cl.push_back("/usr/local/share/hathor-20120128");
    s_i=0;
    this->l=l;
    loadPlugins_continue();
}

void HPlayer::loadPlugins_continue() {
    if(s_i>=cl.size()) {
        emit doneLoadingPlugins();
        return;
    }
    pluginsDir = QDir(cl[s_i]);
    pluginsDir.cd("plugins");

    s_j=0;
    subfiles=pluginsDir.entryList(QDir::Files);
    loadPlugins_continue_continue();
    s_i++;
}

void HPlayer::loadPlugins_continue_continue() {
    if(s_j>=subfiles.size()) {
        QTimer::singleShot(0,this,SLOT(loadPlugins_continue()));
        return;
    }

    QString fileName=subfiles[s_j];
    if(loaded.contains(fileName)) {
        qDebug()<<"WARNING::"<<fileName<<"already loaded. IGNORING!";
        QTimer::singleShot(0,this,SLOT(loadPlugins_continue_continue()));
    } else {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            loaded.push_back(fileName);
            HPlugin* pl=qobject_cast<HPlugin*>(plugin);
            qDebug()<<plugin<<pl;
            if(pl) {
                HPluginManager::singleton()->regPlugin(pl);
                HAbstractTrackProvider* p=pl->trackProvider();
                qDebug()<<p<<"!";
                if(p) {
                    installProvider(p);
                }

                QWidget* lw=pl->initWidget();
                if(lw) {
                    l->addWidget(lw);
                    connect(lw,SIGNAL(destroyed()),this,SLOT(loadPlugins_continue_continue()));
                } else {
                    QTimer::singleShot(0,this,SLOT(loadPlugins_continue_continue()));
                }
            } else {
                QTimer::singleShot(0,this,SLOT(loadPlugins_continue_continue()));
            }
        } else {
            qDebug()<<loader.errorString();
            QTimer::singleShot(0,this,SLOT(loadPlugins_continue_continue()));
        }
    }

    ++s_j;
}

void HScrobbler::onSongStart(HTrack *t) { //NO EVENT LOOP{
    if(s_cur&&s_heuristic.secsTo(QTime::currentTime())>30) {     //TODO::CACHE
        QMap<QString, QString> params;
        params["method"] = "track.scrobble";
        params["timestamp"] = QString::number(QDateTime::currentDateTime().toTime_t());
        params["artist"] = s_cur->getArtistName();
        params["track"] = s_cur->getTrackName();

        QNetworkReply* reply = lastfmext_post( params );
        connect(reply,SIGNAL(finished()),reply,SLOT(deleteLater()));    //!!?
    }
    s_heuristic=QTime::currentTime();
    s_cur=t;
    if(t) {
        QMap<QString, QString> params;
        params["method"] = "track.updateNowPlaying";
        params["artist"] = s_cur->getArtistName();
        params["track"] = s_cur->getTrackName();

        QNetworkReply* reply = lastfmext_post( params );
        connect(reply,SIGNAL(finished()),reply,SLOT(deleteLater()));    //!!?
    }
}
