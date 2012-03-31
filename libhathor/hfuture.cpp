#include "hfuture.h"
#include "lastfmext.h"
#include "hnettloger.h"

#include <QTimer>
#include <QPointer>
#include <QPointer>
#include <QSettings>
#include <QApplication>
#include <QDomDocument>
#include <QDesktopServices>
#include <QFile>
#include <QHttp>
#include <QDir>
#include <QCryptographicHash>

QSettings HCachedInfo::_sett_("Nettek","Hathor");
QHash<QString,QVariant> HCachedInfo::sett;
bool HCachedInfo::_opened=0;

int HCachedInfo::ss_connections=0;
int HCachedPixmap::ss_connections=0;
QList< QPair<QObject*, QString> > HCachedInfo::ss_futureConnetions;
int HCachedPixmap::s_count=0;
QList< QPair<QObject*, QString> > HCachedPixmap::ss_futureConnetions;

QHash<QUrl,HCachedPixmap*> HCachedPixmap::s_map;


HCachedPixmap::HCachedPixmap(const QUrl &url) : pix(0), s_cleared(0), s_proc(0) {
    this->url=url;
    tryAgain=1;
    if(!url.isValid()) {
        qDebug()<<"XX";
    }
    download();
}

void HCachedPixmap::removeFromQueue(QObject *o) {
    for(int i=0;i<queue.size();i++) {
        if(queue[i].first==o) {
            queue.removeAt(i);
            --i;
        }
    }
}

void HCachedPixmap::download() {
    if(s_proc) return;
    s_cleared=0;
    s_proc=1;
    if(file.isOpen()) file.close();
    if(!url.isValid()) url="http://cdn.last.fm/flatness/catalogue/noimage/2/default_artist_mega.png";
    t=QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/hathorMP";
    if(!QFile::exists(t)) {
        QDir r=QDir::root();
        r.mkpath(t);
    }
    QString x=url.toString();
    QString y=x;
    y.remove(0,y.lastIndexOf('.'));
    t+="/"+ QCryptographicHash::hash(url.path().toLocal8Bit(),QCryptographicHash::Md5).toHex()+y;
    file.setFileName(t);
    if(!QFile::exists(t)) {
        if(ss_connections>3) {
            ss_futureConnetions.push_back(qMakePair((QObject*)this,QString("download")));
            return;
        }
        ++ss_connections;
        file.open(QIODevice::WriteOnly);
        file.write("Test");

        http.setHost(url.host(), url.port(80));
        http.get(url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority), &file);
        connect(&http,SIGNAL(done(bool)),this,SLOT(processDownload(bool)));
    } else {
        QMetaObject::invokeMethod(this,"processDownload",Qt::QueuedConnection);
    }
}

void HCachedPixmap::processDownload(bool err) {
    if(err) {
        qDebug()<<http.errorString()<<this;
    }
    ++s_count;
    s_proc=0;

    if(file.isOpen()) {
        file.flush();
        file.close();
    }
    file.setFileName(t);

    file.open(QIODevice::ReadOnly);
    --ss_connections;

    if(ss_connections<4&&ss_futureConnetions.size()) {
        int ti=0, tp=0;
        for(int i=0;i<ss_futureConnetions.size();i++) {
            if(dynamic_cast<HCachedPixmap*>(ss_futureConnetions[i].first)->getTruePriority()>tp) {
                tp=dynamic_cast<HCachedPixmap*>(ss_futureConnetions[i].first)->getTruePriority();
                ti=i;
            } else if(1>tp) {
                tp=1;
                ti=i;
            }
        }
        QPair<QObject*, QString> t=ss_futureConnetions.takeAt(ti);
        ++ss_connections;
        QMetaObject::invokeMethod(t.first,t.second.toUtf8().data(),Qt::QueuedConnection);
    }


    file.waitForReadyRead(9000);

    QMutexLocker locker(&m); Q_UNUSED(locker);  //DO NOT USE QMetaObject::invokeMethod()
    pix=new QImage(QImage::fromData(file.readAll()));
//    pix->loadFromData(file.readAll());
//    delete pix;
//    pix=new QImage(1,1);
//    pix->fill(Qt::red);
    if((pix->isNull()||!pix->width())) {
        delete pix; pix=0; if(!tryAgain) url=""; tryAgain=0; if(file.fileName().size()) file.remove(); download();
    }
    else QTimer::singleShot(0,this,SLOT(processDownload_2()));

    file.close();
}

void HCachedPixmap::processDownload_2() {
    qApp->processEvents();    // Make sure all the objects are sent if they are going to be sent!

    if(!queue.size()) return;
    if(getTruePriority()) {
        if(pix->isNull()) {
            qDebug()<<"WHO ARE YOU TRYING TO TRICK?";
            queue.clear();
        }
        while(queue.size()) {
            QPair<QObject*, QString> p=queue.takeFirst();
            QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Q_ARG(QImage&,*pix));
        }
    } else {
//        qDebug()<<"Clearing unloved queue.";
        queue.clear();
    }
    release();
}


void HCachedPixmap::release_exceptMe() {
    for(int i=0;i<s_map.size();i++) {
        if(s_map.values()[i]->pix&&(s_map.values()[i]!=this)&&!s_map.values()[i]->getTruePriority()&&!s_map.values()[i]->queue.size()/*&&!s_proc*/) {
//            qDebug()<<"DEL"<<s_map.values()[i]->url<<"W/ PRIORITY"<<s_map.values()[i]->getTruePriority()<<s_map.values()[i]->pix;
            s_count--;
            s_map.values()[i]->s_cleared=1;
            delete s_map.values()[i]->pix;
            s_map.values()[i]->pix=0;
        }
    }
}

void HCachedPixmap::release() {
    for(int i=0;i<s_map.size();i++) {
        if(s_map.values()[i]->pix&&/*(s_map.values()[i]!=this)&&*/!s_map.values()[i]->getTruePriority()&&!s_map.values()[i]->s_cleared&&!s_map.values()[i]->queue.size()/*&&!s_proc*/) {
//            qDebug()<<"DEL"<<s_map.values()[i]->url<<"W/ PRIORITY"<<s_map.values()[i]->getTruePriority()<<s_map.values()[i]->pix;
            s_count--;
            s_map.values()[i]->s_cleared=1;
            delete s_map.values()[i]->pix;
            s_map.values()[i]->pix=0;
        }
    }
}

template<> const char* name<int>() { return "int"; }
template<> const char* name<double>() { return "double"; }
template<> const char* name<bool>() { return "bool"; }
template<> const char* name<QString>() { return "QString"; }
template<> const char* name<QStringList>() { return "QStringList"; }
template<> const char* name< QList<double> >() { return "QList<double>"; }
template<typename T> const char* name() { return "UNKNOWN_TYPE_HFUTURE_CPP"; }

void HCachedInfo::sendData() {
    if(!_opened) {
        sett=_sett_.value("MASTER").toHash();
        _opened=1;
    }
    QString desc;
    if(params.contains("track")) desc+="track="+params["track"]+" ";
    if(params.contains("album")) desc+="album="+params["album"]+" ";
    if(params.contains("artist")) desc+="artist="+params["artist"]+" ";

    QMutexLocker lock(&mutex);  //DO NOT USE INVOKEMETHOD!!
    Q_UNUSED(lock);

    if(getting) connect(getting,SIGNAL(notify()),this,SLOT(sendData_processQueue()));
    else if(got) QTimer::singleShot(0,this,SLOT(sendData_processQueue()));        // DO NOT RECURSE
    else {
        bool ok=1;
        for(int i=0;i<data.size();i++) {
            if(!sett.contains(data.values().at(i)->settingsName)) { ok=0; break; }
        }
        if(ok) {
            for(int i=0;i<data.size();i++) {
                AbstractDatum* ad=data.values().at(i);

                Datum<int>* id=dynamic_cast< Datum<int>* >(ad);
                if(id) { id->data=sett.value(id->settingsName).toInt(); continue; }
                Datum<bool>* bd=dynamic_cast< Datum<bool>* >(ad);
                if(bd) { bd->data=sett.value(bd->settingsName).toBool(); continue; }
                Datum<QString>* sd=dynamic_cast< Datum<QString>* >(ad);
                if(sd) { sd->data=sett.value(sd->settingsName).toString(); continue; }
                Datum<QStringList>* sld=dynamic_cast< Datum<QStringList>* >(ad);
                if(sld) { sld->data=sett.value(sld->settingsName).toStringList(); continue; }

                Datum<double>* dd=dynamic_cast< Datum<double>* >(ad);
                if(dd) { dd->data=sett.value(dd->settingsName).toDouble(); continue; }

                Datum<QList<double> >* sd2=dynamic_cast< Datum<QList<double> >* >(ad);
                if(sd2) {
                    QVariantList vl=sett.value(sd2->settingsName).toList();
                    for(int i=0;i<vl.size();i++) sd2->data.push_back(vl[i].toDouble());
                    continue;
                }

                Q_ASSERT(0);
                qDebug()<<"Invalid property type.";
            }
            got=1;  //!!
            QTimer::singleShot(0,this,SLOT(sendData_processQueue())); // DO NOT RECURSE
            return;
        }

        getting=new HRunOnceNotifier;
        if(ss_connections<3) {
            ++ss_connections;
            activate();
        } else {
            ss_futureConnetions.push_back(qMakePair((QObject*)this,(QString)"activate"));
        }
    }
}

void HCachedInfo::activate() {
    QNetworkReply* reply = lastfmext_post( params );
    connect(reply,SIGNAL(finished()),this,SLOT(sendData_process()));
}

void HCachedInfo::sendData_process() {
    if(!_opened) {
        sett=_sett_.value("MASTER").toHash();
        _opened=1;
    }

    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);

    QNetworkReply* reply=qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError||!process( QString::fromUtf8(reply->readAll()) )) {
        --ss_connections;
        QByteArray ba=reply->readAll();

        QString desc;
        if(params.contains("track")) desc+="track="+params["track"]+" ";
        if(params.contains("album")) desc+="album="+params["album"]+" ";
        if(params.contains("artist")) desc+="artist="+params["artist"]+" ";

        qDebug()<<"GOT ERROR - INVALID DATA RECORDED!";
        getting->emitNotify();
        getting=0;
        qDebug()<<ba;
        if(s_tryOnceMore&&(!ba.size()||ba.contains("Rate Limit Exceded"))) {
            s_tryOnceMore=0;
            QTimer::singleShot(0,this,SLOT(sendData()));
        }
        reply->deleteLater();
        return;
    }

    for(int i=0;i<data.size();i++) {
        AbstractDatum* ad=data.values().at(i);

        Datum<int>* id=dynamic_cast< Datum<int>* >(ad);
        if(id) { sett.insert(id->settingsName,id->data); continue; }
        Datum<QString>* sd=dynamic_cast< Datum<QString>* >(ad);
        if(sd) { sett.insert(sd->settingsName,sd->data); continue; }
        Datum<bool>* bd=dynamic_cast< Datum<bool>* >(ad);
        if(bd) { sett.insert(bd->settingsName,bd->data); continue; }
        Datum<QStringList>* sld=dynamic_cast< Datum<QStringList>* >(ad);
        if(sld) { sett.insert(sld->settingsName,sld->data); continue; }
        Datum<double>* dd=dynamic_cast< Datum<double>* >(ad);
        if(dd) { sett.insert(dd->settingsName,dd->data); continue; }
        Datum<QList<double> >* sd2=dynamic_cast< Datum<QList<double> >* >(ad);
        if(sd2) {
            QVariantList vl;
            for(int i=0;i<sd2->data.size();i++) vl.push_back(sd2->data[i]);
            sett.insert(sd2->settingsName,vl); continue;
        }
        Q_ASSERT(0);
        qDebug()<<"Invalid property type.";
    }
    got=1;    // !!
    getting->emitNotify();
    getting=0;

    --ss_connections;
    if(ss_connections<4&&ss_futureConnetions.size()) {
        int ti=0, tp=-1;
        for(int i=0;i<ss_futureConnetions.size();i++) {
            if(dynamic_cast<HCachedInfo*>(ss_futureConnetions[i].first)->getTruePriority()>tp) {
                tp=dynamic_cast<HCachedInfo*>(ss_futureConnetions[i].first)->getTruePriority();
                ti=i;
            }
        }
        QPair<QObject*, QString> t=ss_futureConnetions.takeAt(ti);
        ++ss_connections;
        QMetaObject::invokeMethod(t.first,t.second.toUtf8().data(),Qt::QueuedConnection);
    }
    QTimer::singleShot(0,this,SLOT(sendData_processQueue())); // DO NOT RECURSE
    reply->deleteLater();
}

void HCachedInfo::sendData_processQueue() {
    QString desc;
    if(params.contains("track")) desc+="track="+params["track"]+" ";
    if(params.contains("album")) desc+="album="+params["album"]+" ";
    if(params.contains("artist")) desc+="artist="+params["artist"]+" ";

    if(!got) return;
    for(int i=0;i<data.size();i++) {
        data.values()[i]->send();
    }
}

void HCachedInfo::save() {
    _sett_.setValue("MASTER",sett);
}
