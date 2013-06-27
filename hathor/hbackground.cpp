#include "hbackground.h"
#include <lastfm/ws.h>
#include <lastfm/User.h>
#include <lastfm/Artist.h>
#include <lastfm/AbstractType.h>
#include <QGraphicsScene>
#include <QHttp>
#include <QFile>
#include <QTemporaryFile>
#include "hmaincontext.h"
#include <QTimer>
#include <QFont>
#include <QDir>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QGraphicsView>
#include <QScrollBar>
#include "hartistcontext.h"
#include "hmaincontextoptions.h"
#include <iostream>

int _l=0;
bool ArtistAvatar::_ready = 0;

HBackground::HBackground(QGraphicsScene *sc) : s_mco(0), s_mode(Top), s_style(Album), s_gotTop(0), s_gotRec(0), s_stopRequest(0), s_showingStuff(0) {
    this->_sc=sc;
    QTimer::singleShot(0,this,SLOT(showStuff()));
}

void HBackground::showStuff() {
    s_showingStuff=1;
    _sc->setBackgroundBrush(QBrush(QColor("black")));
    ArtistAvatar::_ready=1;

    closeMode();

    if(!s_mco) {
        s_mco=new HMainContextOptions;
        s_mco->setMinimumWidth(1024);
        QGraphicsProxyWidget* pw=_sc->addWidget(s_mco);
        s_mco->show();
        pw->setPos(183,-305);
        connect(s_mco,SIGNAL(localMode()),this,SLOT(onLocalMode()));
        connect(s_mco,SIGNAL(suggMode()),this,SLOT(onSuggMode()));
        connect(s_mco,SIGNAL(topMode()),this,SLOT(onTopMode()));

        connect(s_mco,SIGNAL(albumMode()),this,SLOT(onAlbumMode()));
        connect(s_mco,SIGNAL(listMode()),this,SLOT(onListMode()));
        connect(s_mco,SIGNAL(playMode()),this,SLOT(onPlayMode()));
    }

    list.clear();
    if(s_mode==Top) {
        if(s_gotTop) {
            list=s_toplist;
            showStuff_makeList();
        } else {
            QMap<QString, QString> p1;
            p1["method"] = "library.getArtists";
            p1["user"] = lastfm::ws::Username;
//            p1["period"]="3month";
            p1["limit"]="5000";
            QNetworkReply* reply = lastfm::ws::get( p1 );

            connect(reply,SIGNAL(finished()),this,SLOT(showStuff_makeList()));
        }
    } else if(s_mode==Suggestions) {
        if(s_gotRec) {
            list=s_reclist;
            showStuff_makeList();
        } else {
            QMap<QString, QString> p1;
            p1["method"] = "user.getRecommendedArtists";
            p1["limit"]="5000";
            QNetworkReply* reply = lastfmext_post(p1);
            connect(reply,SIGNAL(finished()),this,SLOT(showStuff_makeList()));
        }
    } else {
        s_showingStuff=0;
        if(s_stopRequest) {
            doStopRequest();
        }
        return;
    }
}

void HBackground::showStuff_makeList() {
    if(s_stopRequest) {
        s_showingStuff=0;
        doStopRequest();
        delete sender();
        return;
    }

    QNetworkReply* reply=dynamic_cast<QNetworkReply*>(sender());
    if(reply) {
        if(s_mode==Top) {
            list=lastfm::Artist::list( reply );
            s_toplist=list;
            s_gotTop=1;
        } else if(s_mode==Suggestions) {
            list.clear();
            list=lastfm::Artist::list( reply );
            for(int i=0;i<list.size();i++) {
                for(int j=i+1;j<list.size();j++) {
                    if(list[i]==list[j]) {
                        list.removeAt(j--);
                    }
                }
            }
//            for(int i=0;i<list2.size();i++) {
//                for(int j=0;j<list.size();j++) {
//                    if(list2[i]==list[j]) {
//                        list.removeAt(j--);
//                    }
//                }
//            }
            s_reclist=list;
            s_gotRec=1;
        }
        reply->deleteLater();
    }
    w=0;
    x=500;

    _nv.clear();
    _nX.clear();

    x_.clear();
    for(int i=0;i<20;i++) l_[i].clear();
    s_drawingI=0;
    if (s_style==List) {
        qSort(list.begin(),list.end());
        int Cy=-250;
        for(int i=0;i<list.size();i++) {
            ArtistLabel* al=new ArtistLabel(HArtist::get(list[i].name()));
            s_cache.push_back(al);
            connect(al,SIGNAL(showContext()),this,SLOT(showContext()));
            al->setPos(237,Cy);
            _sc->addItem(al);
            Cy+=50;
        }
        s_showingStuff=0;
        if(s_stopRequest) {
            doStopRequest();
            return;
        }
    } else if(s_style==Play) {
        s_showingStuff=0;
        if(s_stopRequest) {
            s_showingStuff=0;
            doStopRequest();
            return;
        }
    } else if(s_style==Album) {
        while(list.size()>61) {
            list.pop_back();
        }
        s_showingStuff=0;
        if(s_stopRequest) {
            s_showingStuff=0;
            doStopRequest();
            return;
        }
        curtime=QTime::currentTime();
        l=0;
        maxY=-305;

        continueShowStuff();
    }
}

void HBackground::continueShowStuff() {
    if(s_stopRequest) {
        s_showingStuff=0;
        doStopRequest();
        return;
    }

    if(s_style==Album) {
        if(s_drawingI>=list.size()) {
            s_showingStuff=0;
            return;
        } else {
            s_priorities.push_back(HCachedPixmap::get(list[s_drawingI].imageUrl(lastfm::AbstractType::LargeImage))->send(this,"showStuff_addPic"));
            **s_priorities.back()=1;
        }
    }
}

void HBackground::showStuff_addPic(QImage& pix) {
    if(!_sc->views()[0]->isVisible()) {
        s_showingStuff=0;
        return;
    }
    if(s_stopRequest) {
        s_showingStuff=0;
        doStopRequest();
        return;
    }

    if(pix.width()!=126) pix=pix.scaledToWidth(126);

    int COLUMN=0;
    int& i=s_drawingI;
    if(!pix.height()) {
        pix=QImage(126,200,QImage::Format_ARGB32);
        pix.fill(Qt::red);
    }

    if(!w){_nv.push_back(0);_nX.push_back(l);}
    ArtistAvatar*fp;
    if(i<8) { COLUMN=i%x; }
    else {
        int minHEIGHT=99999999;
        int minVAL;
        for(int Ci=0;Ci<_nv.size();Ci++) {
            if(_nv[Ci]<minHEIGHT) {
                minVAL=Ci;
                minHEIGHT=_nv[Ci];
            }
        }
        COLUMN=minVAL;
    }
    if(!w) {
        fp=new ArtistAvatar(_sc,list[i].name(),ArtistAvatarList());
        x_.push_back(fp);
    } else {
        ArtistAvatarList aal;
        aal._aa.push_back(x_[COLUMN]);
        fp=new ArtistAvatar(_sc,list[i].name(),aal);
        x_[COLUMN]=fp;
    }

    fp->setPixmap(QPixmap::fromImage(pix));
    fp->setPos(197+_nX[COLUMN],-250+_nv[COLUMN]);
    _sc->addItem(fp);
    s_cache.push_back(fp);

    QPropertyAnimation* anim=new QPropertyAnimation(fp, "echoOpacity");
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(1000);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    _nv[COLUMN]+=pix.height();
    l+=pix.width();
    if(l>1000){if(w==0){x=i+1;}l=0;w+=100;}
    if(COLUMN>=20) {

    }
    l_[COLUMN].push_back(fp);

    maxY=qMax(maxY,_nv[COLUMN]-250);
    connect(fp,SIGNAL(showContext()),this,SLOT(showContext()));

    for(int ik=1;ik<8;ik++) {
        int h1A=0;
        for(int j=0;j<l_[ik].size();j++) {
            int h1B=h1A+l_[ik][j]->pixmap().height();
            int h2A=0;
            for(int k=0;k<l_[ik-1].size();k++) {
                int h2B=h2A+l_[ik-1][k]->pixmap().height();
                if((h2A<=h1A&&h2B>=h1A)||(h1A>=h2A&&h1A<=h2B)||(h2B>=h1A&&h2B<=h1B)||(h1B>=h2B&&h1A<=h2B)||(h1A>=h2A&&h1A<=h2B)||(h2A>=h1A&&h1B>=h2A&&h2B>=h1B)) {
                    l_[ik][j]->addLeft(l_[ik-1][k]);
                }
                h2A=h2B;
            }
            h1A=h1B;
        }
    }

    ++i;
    QTimer::singleShot(0,this,SLOT(continueShowStuff()));
}

void HBackground::showContext() {
    ArtistAvatar* a=dynamic_cast<ArtistAvatar*>(sender());
    ArtistLabel* al=dynamic_cast<ArtistLabel*>(sender());

    Q_ASSERT(a||al);
    if(a) emit showContext(a->s_rep);
    if(al) emit showContext(al->s_rep);
}


void HBackground::doStopRequest() {
    closeMode();
    s_stopRequest=0;
    showStuff();
}

void HBackground::onSuggMode() {
    s_mode=Suggestions;
    openMode();
}

void HBackground::onLocalMode() {
    s_mode=Local;
    openMode();
}

void HBackground::onTopMode() {
    s_mode=Top;
    openMode();
}


void HBackground::onAlbumMode() {
    s_style=Album;
    openMode();
}

void HBackground::onListMode() {
    s_style=List;
    openMode();
}

void HBackground::onPlayMode() {
    s_style=Play;
    openMode();
}

void HBackground::closeMode() {
    ArtistAvatar::_trap.clear();
    while(s_priorities.size()) {
        **s_priorities.takeFirst()=0;
    }

    while(s_cache.size()) {
        _sc->removeItem(s_cache.first());
        delete s_cache.takeFirst();
    }
    HCachedPixmap::release();
}

void HBackground::openMode() {
    if(s_showingStuff) {
        s_stopRequest=1;
        return;
    } else {
        showStuff();
    }
}
