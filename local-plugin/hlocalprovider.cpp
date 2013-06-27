/****************************************************
hlocalprovider.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2012

                 All rights reserved.
*****************************************************/

#include "hlocalprovider.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <phonon/MediaObject>
#include <QtPlugin>
#include <lastfm/ws.h>
#include <lastfm/User.h>
#include <lastfm/Artist.h>
#include <QInputDialog>
#include <QFileDialog>
#include <QDirIterator>

QString HLocalProvider::local_standardized(QString r) {
    r=r.toLower();
    r.replace("&amp;","&");
    r.replace("#39;","\'");
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

HLocalProvider* HLocalProvider::s_singleton=0;

HLocalProvider::HLocalProvider() : s_settings("Nettek","Local Plugin for Hathor") {
    s_singleton=this;
    if(s_settings.value("Directories").isNull()) {
        s_intro=new HLocalIntro(true,this);
        connect(s_intro,SIGNAL(destroyed()),this,SLOT(byeByeIntro()));
    } else {
        s_intro=new HLocalIntro(false,this);
        connect(s_intro,SIGNAL(destroyed()),this,SLOT(byeByeIntro()));
        s_intro->go();
    }
}

QString HLocalProvider::getKey(HTrack& track) {
    QString l=local_standardized(track.getArtistName()+"__"+track.getTrackName());
    if(s_theGreatHash.contains(l)) {
        qDebug()<<"LOCAL!!"<<s_theGreatHash[l];
        return s_theGreatHash[l].toString();
    } else return "_NO_RESULT_";
}

Q_EXPORT_PLUGIN2(hlocal_plugin, HLocalPlugin)
