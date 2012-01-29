#include "habstractmusicinterface.h"
#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QPluginLoader>

HPlayer* HPlayer::s_singleton=0;

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
    connect(Q,SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SIGNAL(stateChanged(HAbstractTrackInterface::State)));
    connect(Q,SIGNAL(trackChanged(HTrack&)),this,SIGNAL(trackChanged(HTrack&)));
    connect(Q,SIGNAL(shuffleToggled(bool)),this,SIGNAL(shuffleToggled(bool)));
    return sq;
}

void HStandardQueue::queue(HTrack *track) {
    HPlayer_PotentialTrack* pt=new HPlayer_PotentialTrack(*track);
    s_queue.push_back(pt);
    for(int i=0;i<s_providers.size();i++) {
        pt->regProvider(s_providers[i]);
    }
    if(!s_currentTrack) playNext();
}

void HStandardQueue::queue(HAlbum* album) {
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
        qDebug()<<"ABOUT TO PLAY!";
        s_currentTrack->play();
    } else {
        s_currentTrack=0;
        emit stateChanged(HAbstractTrackInterface::Stopped);
    }
}

void HPlayer::playNext() {
    if(Q) Q->playNext();
}

void HPlayer::loadPlugins() {
    QStringList cl;
    cl.push_back(qApp->applicationDirPath());
    cl.push_back("/usr/share/hathor-20120128");
    cl.push_back("/usr/local/share/hathor-20120128");
    QStringList loaded;
    for(int i=0;i<cl.size();i++) {
        QDir pluginsDir = QDir(cl[i]);
        pluginsDir.cd("plugins");

        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (plugin&&!loaded.contains(fileName)) {
                qDebug()<<"Loading plugin"<<fileName;
                loaded.push_back(fileName);
                HAbstractTrackProvider* p=qobject_cast<HAbstractTrackProvider*>(plugin);
                if(p) {
                    installProvider(p);
                }
            } else if(loaded.contains(fileName)) {
                qDebug()<<"WARNING::"<<fileName<<"already loaded. IGNORING!";
            }
        }
    }
}
