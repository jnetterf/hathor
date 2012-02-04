#include "hfuture.h"
#include "lastfmext.h"

#include <QTimer>
#include <QSettings>
#include <QDomDocument>
#include <QDesktopServices>
#include <QFile>
#include <QHttp>
#include <QDir>
#include <QCryptographicHash>

QSettings HCachedInfo::sett("Nettek","Hathor");

int HCachedInfo::ss_connections=0;
int HCachedPixmap::ss_connections=0;
QList< QPair<QObject*, QString> > HCachedInfo::ss_futureConnetions;

HCachedPixmap::HCachedPixmap(const QUrl &url) {
    this->url=url;
    download();
    tryAgain=1;
}

void HCachedPixmap::download() {
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
    if(!QFile::exists(t)) {
        if(ss_connections>3) {
            QTimer::singleShot(400,this,SLOT(download()));
            return;
        }
        ++ss_connections;
        file.setFileName(t);
        file.open(QIODevice::WriteOnly);

        http.setHost(url.host(), url.port(80));
        http.get(url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority), &file);
        connect(&http,SIGNAL(done(bool)),this,SLOT(processDownload()));
    } else processDownload();
    tryAgain=0;
}

void HCachedPixmap::processDownload(bool err) {
    if(file.isOpen()) {
        file.flush();
        file.close();
    }
    --ss_connections;
    QMutexLocker locker(&m); Q_UNUSED(locker);  //DO NOT USE QMetaObject::invokeMethod()
    pix.load(t);
    if(!pix.width()&&tryAgain) { QFile::remove(t); download(); }
    else QTimer::singleShot(0,this,SLOT(processDownload_2()));
}

void HCachedPixmap::processDownload_2() {
//    QMutexLocker locker(&m); Q_UNUSED(locker);
    while(queue.size()) {
        m.lock();
        QPair<QObject*, QString> p=queue.takeFirst();
        m.unlock();
        QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),Q_ARG(QPixmap,pix));
    }
    queue.clear();
}

template<> const char* name<int>() { return "int"; }
template<> const char* name<double>() { return "double"; }
template<> const char* name<bool>() { return "bool"; }
template<> const char* name<QString>() { return "QString"; }
template<> const char* name<QStringList>() { return "QStringList"; }
template<> const char* name< QList<double> >() { return "QList<double>"; }
template<typename T> const char* name() { return "UNKNOWN_TYPE_HFUTURE_CPP"; }

void HCachedInfo::sendData() {
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
    QMutexLocker lock(&mutex);
    Q_UNUSED(lock);

    QNetworkReply* reply=qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError||!process( QString::fromUtf8(reply->readAll()) )) {
        --ss_connections;
        QByteArray ba=reply->readAll();
        qDebug()<<"GOT ERROR - INVALID DATA RECORDED!";
        getting->emitNotify();
        getting=0;
        if(!ba.size()||ba.contains("Rate Limit Exceded")) {
            QTimer::singleShot(0,this,SLOT(sendData()));
        }
        return;
    }

    for(int i=0;i<data.size();i++) {
        AbstractDatum* ad=data.values().at(i);

        Datum<int>* id=dynamic_cast< Datum<int>* >(ad);
        if(id) { sett.setValue(id->settingsName,id->data); continue; }
        Datum<QString>* sd=dynamic_cast< Datum<QString>* >(ad);
        if(sd) { sett.setValue(sd->settingsName,sd->data); continue; }
        Datum<bool>* bd=dynamic_cast< Datum<bool>* >(ad);
        if(bd) { sett.setValue(bd->settingsName,bd->data); continue; }
        Datum<QStringList>* sld=dynamic_cast< Datum<QStringList>* >(ad);
        if(sld) { sett.setValue(sld->settingsName,sld->data); continue; }
        Datum<double>* dd=dynamic_cast< Datum<double>* >(ad);
        if(dd) { sett.setValue(dd->settingsName,dd->data); continue; }
        Datum<QList<double> >* sd2=dynamic_cast< Datum<QList<double> >* >(ad);
        if(sd2) {
            QVariantList vl;
            for(int i=0;i<sd2->data.size();i++) vl.push_back(sd2->data[i]);
            sett.setValue(sd2->settingsName,vl); continue;
        }
        Q_ASSERT(0);
        qDebug()<<"Invalid property type.";
    }
    got=1;    // !!
    getting->emitNotify();
    getting=0;

    --ss_connections;
    if(ss_connections<4&&ss_futureConnetions.size()) {
        QPair<QObject*, QString> t=ss_futureConnetions.takeLast();
        ++ss_connections;
        QMetaObject::invokeMethod(t.first,t.second.toUtf8().data(),Qt::QueuedConnection);
    }
    QTimer::singleShot(0,this,SLOT(sendData_processQueue())); // DO NOT RECURSE
}

void HCachedInfo::sendData_processQueue() {
    if(!got) return;
    for(int i=0;i<data.size();i++) {
        data.values()[i]->send();
    }
}
