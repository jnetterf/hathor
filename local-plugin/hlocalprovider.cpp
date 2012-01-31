/****************************************************
hlocalprovider.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2012

                 All rights reserved.
*****************************************************/

#include "hlocalprovider.h"
#include <QDebug>
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>
#include <QSettings>
#include <phonon/MediaObject>
#include <QtPlugin>
#include <lastfm/ws.h>
#include <lastfm/User>
#include <lastfm/Artist>
#include <QInputDialog>
#include <QFileDialog>
#include <QDirIterator>

QString local_standardized(QString r) {
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
    QStringList dirs;
    if(s_settings.value("Directories").isNull()) {
        while(1) {
            if(QMessageBox::information(0,"Local Plugin for Hathor","Press ok to select a directory which has music in it, or cancel to stop searching. (This is temporary)",QMessageBox::Ok,QMessageBox::Cancel)
                    ==QMessageBox::Cancel) break;
            QString a= QFileDialog::getExistingDirectory(0,"Select directory with music");
            if(a.size()) dirs.push_back(a);
        }
        s_settings.setValue("Directories",dirs);
    } else dirs=s_settings.value("Directories").toStringList();

    if(!s_settings.value("The Great Hash").isNull()) s_theGreatHash=s_settings.value("The Great Hash").toHash();
    if(!s_settings.value("The Inverse Hash").isNull()) s_theInverseHash=s_settings.value("The Inverse Hash").toHash();

    // rescan (FIX THIS)

    QStringList files;

    for(int i=0;i<dirs.size();i++) {
        QDirIterator it(dirs[i], QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while(it.hasNext()) {
            it.next();
            if(it.fileInfo().completeSuffix().toLower() == "mp3"||it.fileInfo().completeSuffix().toLower() == "ogg"||it.fileInfo().completeSuffix().toLower() == "wav"||
                    it.fileInfo().completeSuffix().toLower() == "flac") files.push_back(it.filePath());
        }
    }

    Phonon::MediaObject mo;
    for(int i=0;i<files.size();i++) {
        qDebug()<<"Scanning "<<files[i];
        if(s_theInverseHash.contains(files[i])) continue;
        Phonon::MediaSource ms(files[i]);
        mo.setCurrentSource(ms);

        //
        // BEHOLD - THE GREATEST HACK MANKIND HAS EVER WITNESSED: (no really, fix this)
        //

        mo.play();
        mo.pause();
        QEventLoop loop;
        connect(&mo,SIGNAL(metaDataChanged()),&loop,SLOT(quit()));
        QTimer::singleShot(20,&loop,SLOT(quit()));
        loop.exec();
        if(!mo.metaData("ARTIST").size()||!mo.metaData("TITLE").size()) {
            connect(&mo,SIGNAL(metaDataChanged()),&loop,SLOT(quit()));
            QTimer::singleShot(20,&loop,SLOT(quit()));
            loop.exec();
        }
        qDebug()<<mo.metaData();

        if(mo.metaData("ARTIST").size()&&mo.metaData("TITLE").size()) {
            s_theGreatHash.insert(local_standardized(mo.metaData("ARTIST").first()+"__"+mo.metaData("TITLE").first()),files[i]);
            s_theInverseHash.insert(files[i],local_standardized(mo.metaData("ARTIST").first()+"__"+mo.metaData("TITLE").first()));
        }

        mo.stop();
        //
        // End.
        //
    }
    s_settings.setValue("The Great Hash",QVariant::fromValue(s_theGreatHash));
    s_settings.setValue("The Inverse Hash",QVariant::fromValue(s_theInverseHash));
}

QString HLocalProvider::getKey(HTrack& track) {
    QString l=local_standardized(track.getArtistName()+"__"+track.getTrackName());
    if(s_theGreatHash.contains(l)) {
        qDebug()<<"LOCAL!!"<<s_theGreatHash[l];
        return s_theGreatHash[l].toString();
    } else return "_NO_RESULT_";
}

Q_EXPORT_PLUGIN2(hlocal_provider, HLocalProvider)
