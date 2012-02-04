/****************************************************
hlocalprovider.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2012

                 All rights reserved.
*****************************************************/

#ifndef HLOCALPROVIDER_H
#define HLOCALPROVIDER_H

#include "hbrowser.h"
#include "habstractmusicinterface.h"
#include "hlocalintro.h"
#include <phonon/MediaObject>

class HLocalProvider : public QObject, public HAbstractTrackProvider
{
    Q_OBJECT
    Q_INTERFACES(HAbstractTrackProvider)
    friend class HLocalIntro;
    static HLocalProvider* s_singleton;
    QSettings s_settings;
    QVariantHash s_theGreatHash;
    QVariantHash s_theInverseHash;
    HLocalIntro* s_intro;

public:
    static QString local_standardized(QString r);
    HLocalProvider();

public: //HAbstractTrackProvider
    int globalScore() { return 100; }
    void sendScore(HTrack& track,QObject* o,QString m) {
        bool ok=getKey(track)!="_NO_RESULT_";
        if(ok) QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(int,99),Q_ARG(HAbstractTrackProvider*,this));
        else QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(int,0),Q_ARG(HAbstractTrackProvider*,this));
    }
    void sendTrack(HTrack& track,QObject* o,QString m) {
        HAbstractTrackInterface* ti=new HPhononTrackInterface(track,getKey(track));
        QMetaObject::invokeMethod(o,m.toUtf8().data(),Qt::QueuedConnection,Q_ARG(HAbstractTrackInterface*,ti),Q_ARG(HAbstractTrackProvider*,this));
    }

    HAbstractTrackInterface* queue(HTrack& track) { return new HPhononTrackInterface(track,getKey(track)); }

    QWidget* initWidget() { return s_intro; }

    QString name() { return "local"; }

public slots:
    void byeByeIntro() {
        s_intro=0;
    }

private:
    QString getKey(HTrack& track);
public:
    static HLocalProvider* singleton() { Q_ASSERT(s_singleton); return s_singleton; }
    QString search(QString query, QString artistF, QString trackF);
};

#endif // HLOCALINTERFACE_H
