#include "hsearchcontext.h"
#include "ui_hsearchcontext.h"
#include "halbumbox.h"
#include "hartistbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include <QNetworkReply>
#include <QDomDocument>
#include <QPropertyAnimation>
#include <stdexcept>
#include "lastfmext.h"

HSearchContext* HSearchContext::s_singleton;

HSearchContext::HSearchContext(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HSearchContext)
{
    ui->setupUi(this);
    QVBoxLayout* vb;
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_albums->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tracks->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tags->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_artists->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_usersAndGroups->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_venues->setLayout(vb);
}

HSearchContext::~HSearchContext()
{
    delete ui;
}

void HSearchContext::setSearchTerm(QString s) {
    if(s.size()) {
        s_term=s;
        emit searchTermChanged();
        QTimer::singleShot(0,this,SLOT(setSearchTerm(QString)));
        return;
    }

    s=s_term;

    // clear the stuff
    while(ui->widget_albums->layout()->count()) {
        /*delete */ui->widget_albums->layout()->takeAt(0)->widget()->hide();
    }
    while(ui->widget_artists->layout()->count()) {
        /*delete */ui->widget_artists->layout()->takeAt(0)->widget()->hide();
    }
    while(ui->widget_tags->layout()->count()) {
        /*delete */ui->widget_tags->layout()->takeAt(0)->widget()->hide();
    }
    while(ui->widget_tracks->layout()->count()) {
        /*delete */ui->widget_tracks->layout()->takeAt(0)->widget()->hide();
    }
    while(ui->widget_usersAndGroups->layout()->count()) {
        /*delete */ui->widget_usersAndGroups->layout()->takeAt(0)->widget()->hide();
    }
    while(ui->widget_venues->layout()->count()) {
        /*delete */ui->widget_venues->layout()->takeAt(0)->widget()->hide();
    }

    s_albumCount=0;
    s_artistCount=0;
    s_tagCount=0;
    s_trackCount=0;
    s_userCount=0;
    s_venueCount=0;

    if(s_term!=s) return;

    // download the stuff without caching.
    getMoreTracks(s);
    if(s_term!=s) return;
    getMoreArtists(s);
    if(s_term!=s) return;
    getMoreAlbums(s);
    if(s_term!=s) return;
    getMoreTags(s);
    if(s_term!=s) return;
    getMoreUsers(s);
    if(s_term!=s) return;
    getMoreVenues(s);
    if(s_term!=s) return;

    // bye
}

void HSearchContext::getMoreAlbums(QString s) {
    QMap<QString, QString> params;
    params["method"] = "album.search";
    params["album"] = s;
    QNetworkReply* reply = lastfmext_post( params );
    reply->setProperty("s",s);

    connect(reply,SIGNAL(finished()),this,SLOT(getMoreAlbums_2()));
    connect(this,SIGNAL(searchTermChanged()),reply,SLOT(deleteLater()));
}

void HSearchContext::getMoreAlbums_2() {
    QNetworkReply* reply=dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;
    if(reply->error()!=QNetworkReply::NoError) {
        return;
    }
    QString s=reply->property("s").toString();
    if(s_term!=s) {
        delete reply;
        return;
    }

    if(reply->error()!=QNetworkReply::NoError) {
        getMoreAlbums(s);
        return;
    }

    QString album;

    int q=0;

    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "album" && k.nodeName() == "name" ) { album = j.toText().data(); }
                            if ( l.nodeName() == "album" && k.nodeName() == "artist" ) {
                                q++;
                                if(q==8) return;
                                if(s_term!=s) return;
                                ui->widget_albums->layout()->addWidget(HAlbumBox::getBox(HAlbum::get(j.toText().data(),album)));
                                if(s_term!=s) return;
                            }
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
}

void HSearchContext::getMoreArtists(QString s) {
    QMap<QString, QString> params;
    params["method"] = "artist.search";
    params["artist"] = s;
    QNetworkReply* reply = lastfmext_post( params );
    reply->setProperty("s",s);

    connect(reply,SIGNAL(finished()),this,SLOT(getMoreArtists_2()));
    connect(this,SIGNAL(searchTermChanged()),reply,SLOT(deleteLater()));
}

void HSearchContext::getMoreArtists_2() {
    QNetworkReply* reply=dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;
    if(reply->error()!=QNetworkReply::NoError) {
        return;
    }
    QString s=reply->property("s").toString();
    if(s_term!=s) {
        delete reply;
        return;
    }

    if(reply->error()!=QNetworkReply::NoError) {
        getMoreAlbums(s);
        return;
    }

    int q=0;

    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "artist" && k.nodeName() == "name" ) {
                                q++;
                                if(q==8) return;
                                if(s_term!=s) return;
                                HArtistBox* ab=HArtistBox::getBox(HArtist::get(j.toText().data()));
                                QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
                                pa->setStartValue(0);
                                pa->setEndValue(ab->sizeHint().height());
                                pa->setDuration(500);
                                pa->start(QAbstractAnimation::DeleteWhenStopped);
                                ui->widget_artists->layout()->addWidget(ab);

                                if(s_term!=s) return;
                            }
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
}

void HSearchContext::getMoreTags(QString s) {
    QMap<QString, QString> params;
    params["method"] = "tag.search";
    params["tag"] = s;
    QNetworkReply* reply = lastfmext_post( params );
    reply->setProperty("s",s);

    connect(reply,SIGNAL(finished()),this,SLOT(getMoreTags_2()));
    connect(this,SIGNAL(searchTermChanged()),reply,SLOT(deleteLater()));
}

void HSearchContext::getMoreTags_2() {
    QNetworkReply* reply=dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;
    if(reply->error()!=QNetworkReply::NoError) {
        return;
    }
    QString s=reply->property("s").toString();
    if(s_term!=s) {
        delete reply;
        return;
    }

    if(reply->error()!=QNetworkReply::NoError) {
        getMoreTags(s);
        return;
    }

    int q=0;

    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "tag" && k.nodeName() == "name" ) {
                                q++;
                                if(q==8) return;
                                if(s_term!=s) return;
                                ui->widget_tags->layout()->addWidget(HTagBox::getBox(HTag::get(j.toText().data())));
                                if(s_term!=s) return;
                            }
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
}

void HSearchContext::getMoreTracks(QString s) {
    QMap<QString, QString> params;
    params["method"] = "track.search";
    params["track"] = s;
    QNetworkReply* reply = lastfmext_post( params );
    reply->setProperty("s",s);

    connect( reply, SIGNAL(finished()), this, SLOT(getMoreTracks_2()));
    connect(this,SIGNAL(searchTermChanged()),reply,SLOT(deleteLater()));
}

void HSearchContext::getMoreTracks_2() {
    QNetworkReply* reply=dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;
    if(reply->error()!=QNetworkReply::NoError) {
        return;
    }
    QString s=reply->property("s").toString();
    if(s_term!=s) {
        delete reply;
        return;
    }

    QString track;

    int q=0;

    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        for (QDomNode j = k.firstChild(); !j.isNull(); j = j.nextSibling()) {
                            if ( l.nodeName() == "track" && k.nodeName() == "name" ) { track = j.toText().data(); }
                            if ( l.nodeName() == "track" && k.nodeName() == "artist" ) {
                                q++;
                                if(q==8) return;
                                if(s_term!=s) return;

                                HTrackBox* ab=HTrackBox::getBox(HTrack::get(j.toText().data(),track));

                                QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
                                pa->setStartValue(0);
                                pa->setEndValue(32);
                                pa->setDuration(300);
                                pa->start(QAbstractAnimation::DeleteWhenStopped);
                                ui->widget_tracks->layout()->addWidget(ab);

                                if(s_term!=s) return;
                            }
                        }
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
}

void HSearchContext::getMoreUsers(QString s) {

}

void HSearchContext::getMoreUsers_2() {

}

void HSearchContext::getMoreVenues(QString s) {

}

void HSearchContext::getMoreVenues_2() {

}
