#include "hbackground.h"
#include <lastfm/ws.h>
#include <lastfm/User>
#include <lastfm/Artist>
#include <QGraphicsScene>
#include <QHttp>
#include <QFile>
#include <QTemporaryFile>
#include <QEventLoop>
#include "hmaincontext.h"
#include <QTimer>
#include <QFont>
#include <QDir>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QGraphicsView>
#include <QScrollBar>
#include "hartistcontext.h"
#include "hrdioprovider.h"
#include <iostream>

int _l=0;
bool ArtistAvatar::_ready = 0;

static QPixmap download(QUrl url, bool tryAgain=1) {
    if(!url.isValid()) url="http://cdn.last.fm/flatness/catalogue/noimage/2/default_artist_large.png";
    QString t=QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/hathorMP";
    if(!QFile::exists(t)) {
        QDir r=QDir::root();
        r.mkpath(t);
    }

    QString x=url.toString();
    QString y=x;
    y.remove(0,y.lastIndexOf('.'));
    t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+y;

    if(!QFile::exists(t)) {
        QHttp http;
        QEventLoop loop;
        QFile file;
        QObject::connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));

        file.setFileName(t);
        file.open(QIODevice::WriteOnly);

        http.setHost(url.host(), url.port(80));
        http.get(url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority),
                 &file);
        loop.exec();
        file.close();
    }
    QPixmap apix;
    apix.load(t);
    if(!apix.width()&&tryAgain) { QFile::remove(t); download(url,0); }
    return apix;
}

HBackground::HBackground(QGraphicsScene *sc) {
    this->_sc=sc;
    QTimer::singleShot(0,this,SLOT(showStuff()));
}

void HBackground::showStuff() {
    _sc->setBackgroundBrush(QBrush(QColor("black")));
    ArtistAvatar::_ready=1;
    for(int i=_sc->items().size()-1;i>=0;i--)_sc->removeItem(_sc->items()[i]);

    QMap<QString, QString> p1;;
    p1["method"] = "user.getTopArtists";
    p1["user"] = lastfm::AuthenticatedUser().name();
    p1["period"]="3month";

    p1["limit"]="200";
    QNetworkReply* reply = lastfm::ws::get( p1 );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    QList<lastfm::Artist> list=lastfm::Artist::list( reply );
    int w=0,x=500;
    QList<int> _nv;
    QList<int> _nX;

    QList<ArtistAvatar*> x_;
    QList<ArtistAvatar*> l_[20];
    ArtistAvatar* first=NULL;


    QTime curtime=QTime::currentTime();
    int l=0;
    int t=200;
    QSettings sett("hathorMP","global");
    QGraphicsTextItem* ti=_sc->addText("Top Artists",QFont("Candara",30,75));
    PlayLibraryButton* pb=new PlayLibraryButton(list);
    pb->setPos(183,-305);
    _sc->addItem(pb);
    pb->setEchoOpacity(1.0);
    ti->setDefaultTextColor("grey");
    ti->setPos(226,-305);
    QList<HTrack*> topTracks=HUser::get(lastfm::ws::Username).getTopTracks();
    int maxY=-305;
    for(int i=0;i<list.size();i++){
        int COLUMN=0;
//        _sc->setBackgroundBrush(QBrush(QColor(255.0-255.0*(double)i/(list.size()),255.0-255.0*(double)i/(list.size()),255.0-
//                                              255.0*(double)i/(list.size()))));
        QPixmap pix=download(list[i].imageUrl(lastfm::Large));
        if(pix.isNull()) pix=download(list[i].imageUrl(lastfm::Large));   //jic
        if(!pix.height()) {
            pix=QPixmap(126,200);
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
        if(i==6)first=fp;

        fp->setPixmap(pix);
        fp->setPos(197+_nX[COLUMN],-250+_nv[COLUMN]);
        _sc->addItem(fp);

        QPropertyAnimation* anim=new QPropertyAnimation(fp, "echoOpacity");
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setDuration(1000);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        _nv[COLUMN]+=pix.height();
        l+=pix.width();
        if(l>1000){if(w==0){x=i+1;}l=0;w+=100;}
        Q_ASSERT(COLUMN<20);
        l_[COLUMN].push_back(fp);

        maxY=qMax(maxY,_nv[COLUMN]-250);
        connect(fp,SIGNAL(showContext()),this,SLOT(showContext()));

        for(int i=1;i<8;i++) {
            int h1A=0;
            for(int j=0;j<l_[i].size();j++) {
                int h1B=h1A+l_[i][j]->pixmap().height();
                int h2A=0;
                for(int k=0;k<l_[i-1].size();k++) {
                    int h2B=h2A+l_[i-1][k]->pixmap().height();
                    if((h2A<=h1A&&h2B>=h1A)||(h1A>=h2A&&h1A<=h2B)||(h2B>=h1A&&h2B<=h1B)||(h1B>=h2B&&h1A<=h2B)||(h1A>=h2A&&h1A<=h2B)||(h2A>=h1A&&h1B>=h2A&&h2B>=h1B)) {
                        l_[i][j]->addLeft(l_[i-1][k]);
                    }
                    h2A=h2B;
                }
                h1A=h1B;
            }
        }
    }

    for(int i=0;i<20;i++) l_[i].clear();
    x_.clear();
    _nv.clear();
    _nX.clear();
    w=0;

    QList<lastfm::Artist> list2=list;
    {
        QMap<QString, QString> p1;;
        p1["method"] = "user.getRecommendedArtists";
        p1["limit"]="200";
        QNetworkReply* reply = lastfmext_post(p1);

        QEventLoop loop;
        loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
        loop.exec();
        list.clear();
        list=lastfm::Artist::list( reply );
        for(int i=0;i<list.size();i++) {
            for(int j=i+1;j<list.size();j++) {
                if(list[i]==list[j]) {
                    list.removeAt(j--);
                }
            }
        }
        for(int i=0;i<list2.size();i++) {
            for(int j=0;j<list.size();j++) {
                if(list2[i]==list[j]) {
                    list.removeAt(j--);
                }
            }
        }
    }
    ti=_sc->addText("Your Recommendations",QFont("Candara",30,75));
    pb=new PlayLibraryButton(list);
    pb->setPos(183,maxY+30);
    _sc->addItem(pb);
    pb->setEchoOpacity(1.0);
    ti->setDefaultTextColor("grey");
    ti->setPos(226,maxY+30);

    maxY+=90;

    for(int i=0;i<list.size();i++){
        int COLUMN=0;
        QPixmap pix=download(list[i].imageUrl(lastfm::Large));
        if(pix.isNull()) pix=download(list[i].imageUrl(lastfm::Large));   //jic
        if(!pix.height()) {
            pix=QPixmap(126,200);
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
        if(i==6)first=fp;

        fp->setPixmap(pix);
        fp->setPos(197+_nX[COLUMN],maxY+_nv[COLUMN]);
        _sc->addItem(fp);

        QPropertyAnimation* anim=new QPropertyAnimation(fp, "echoOpacity");
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setDuration(1000);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        _nv[COLUMN]+=pix.height();
        l+=pix.width();
        if(l>1000){if(w==0){x=i+1;}l=0;w+=100;}
        Q_ASSERT(COLUMN<20);
        l_[COLUMN].push_back(fp);

        curtime=QTime::currentTime();
        connect(fp,SIGNAL(showContext()),this,SLOT(showContext()));

        for(int i=1;i<8;i++) {
            int h1A=0;
            for(int j=0;j<l_[i].size();j++) {
                int h1B=h1A+l_[i][j]->pixmap().height();
                int h2A=0;
                for(int k=0;k<l_[i-1].size();k++) {
                    int h2B=h2A+l_[i-1][k]->pixmap().height();
                    if((h2A<=h1A&&h2B>=h1A)||(h1A>=h2A&&h1A<=h2B)||(h2B>=h1A&&h2B<=h1B)||(h1B>=h2B&&h1A<=h2B)||(h1A>=h2A&&h1A<=h2B)||(h2A>=h1A&&h1B>=h2A&&h2B>=h1B)) {
                        l_[i][j]->addLeft(l_[i-1][k]);
                    }
                    h2A=h2B;
                }
                h1A=h1B;
            }
        }
    }
    sett.setValue("precached",1);
}

void HBackground::showContext() {
    ArtistAvatar* a=dynamic_cast<ArtistAvatar*>(sender());
    Q_ASSERT(a);
    emit showContext(a->s_rep);
}
