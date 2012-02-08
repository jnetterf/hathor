#ifndef HFUTURE_H
#define HFUTURE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QPair>
#include <QMutex>
#include <QStringList>
#include <QDebug>
#include <QPixmap>
#include <QFile>
#include <QHttp>
#include <QSettings>
#include <QUrl>
#include "libhathor_global.h"
#include "hnotifier.h"

template<class T> const char* name();

class LIBHATHORSHARED_EXPORT HCachedInfo : public QObject {
    Q_OBJECT
    friend class HAlbum;
    static int ss_connections;
    static QList< QPair<QObject*, QString> > ss_futureConnetions;
    static QSettings _sett_;
    static QHash<QString,QVariant> sett;
    static bool _opened;

    QMutex mutex;
    QMap<QString, QString> params;

    struct AbstractDatum {
        QMutex m;
        QString settingsName;
        QList< QPair<QObject*, QString > > queue;
        AbstractDatum(QString csettingsName) : settingsName(csettingsName) {}
        virtual void send()=0;
    };

    template<typename T> struct Datum : AbstractDatum {
        T data;
        void send() {
            while(queue.size()) {
                m.lock();
                QPair<QObject*,QString> p=queue.takeFirst();
                m.unlock();
                QMetaObject::invokeMethod(p.first,p.second.toUtf8().data(),QArgument<T>(name<T>(),data));
            }
        }

        Datum(QString csettingsName) : AbstractDatum(csettingsName) {}
    };

    QHash<QString, AbstractDatum*> data;

    bool got;
    HRunOnceNotifier* getting;
public:
    static void save();
    HCachedInfo() : got(0), getting(0) {}
    void setParams(QMap<QString, QString> cparams) {
        params=cparams;
    }

    template<typename T> void addProperty(QString localName, QString settingsName) {
        Datum<T>* d=new Datum<T>(settingsName+localName);
        data.insert(localName,d);
        if(!strcmp(name<T>(),"int")) {
            reinterpret_cast<int&>(d->data)=0;
        }
    }
    void sendProperty(QString localName,QObject* o, QString m) {
        Q_ASSERT(data.contains(localName));
        if(!data.contains(localName)) return;
        data[localName]->queue.push_back(qMakePair(o,m));
        sendData();
    }
    template<typename T> void setProperty(QString localName, T d) {
        Q_ASSERT(data.contains(localName));
        if(!data.contains(localName)) return;
        dynamic_cast< Datum<T>* >(data[localName])->data=d;
    }

    virtual bool process(const QString& data)=0;

    bool isCached() { return got&&!getting; }

public slots:
    void sendData();
    void activate();
    void sendData_process();
    void sendData_processQueue();
};

class LIBHATHORSHARED_EXPORT HCachedPixmap : public QObject {
    Q_OBJECT
    QList< QPair<QObject*, QString > > queue;
    QPixmap pix;
    QHttp http;
    QFile file;
    QString t;
    QUrl url;
    QMutex m;
    bool tryAgain;
    static int ss_connections;

    HCachedPixmap(const QUrl& url);
    static QHash<QUrl,HCachedPixmap*> s_map;
public:
    static HCachedPixmap* get(const QUrl& url) {
        if(!s_map.contains(url)) s_map[url]=new HCachedPixmap(url);
        return s_map[url];
    }

public slots:
    void send(QObject*o,QString m) { queue.push_back(qMakePair(o,m)); if(!pix.isNull()) processDownload_2();}
    void download();
    void processDownload(bool err=0); /* internal */
    void processDownload_2();
};

#endif // HFUTURE_H
