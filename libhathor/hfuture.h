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

    QHash<QString, QHash<QObject*, int*> > s_priority;

    int getTruePriority() {
        int tp=0;
        for(int i=0;i<s_priority.size();i++) {
            for(int j=0;j<s_priority.values()[i].size();j++) {
                if(s_priority.values()[i].values()[j]) {
                    tp=qMax(*s_priority.values()[i].values()[j],tp);
                }
            }
        }
        return tp;
    }

    struct AbstractDatum {
        QMutex m;
        QString settingsName;
        QList< QPair<QObject*, QString > > queue;
        AbstractDatum(QString csettingsName) : settingsName(csettingsName) {}

        void removeFromQueue(QObject*o) {
            for(int i=0;i<queue.size();i++) {
                if(queue[i].first==o) {
                    queue.removeAt(i);
                    --i;
                }
            }
        }

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

    int** sendProperty(QString localName,QObject* o, QString m,QObject* guest=0) {
        Q_ASSERT(data.contains(localName));
        connect(o,SIGNAL(destroyed(QObject*)),this,SLOT(removeFromQueue(QObject*)));
        if(!s_priority[localName].contains(guest?guest:o)) s_priority[localName][guest?guest:o]=new int(0);
        if(!data.contains(localName)) return &s_priority[localName][guest?guest:o];
        data[localName]->queue.push_back(qMakePair(o,m));
        sendData();
        return &s_priority[localName][guest?guest:o];
    }

    int** getPriorityForProperty(QObject* o,QString p) {
        return &s_priority[p][o];
    }

    template<typename T> void setProperty(QString localName, T d) {
        Q_ASSERT(data.contains(localName));
        if(!data.contains(localName)) return;
        dynamic_cast< Datum<T>* >(data[localName])->data=d;
    }

public slots:
    void removeFromQueue(QObject* b) {
        for(int i=0;i<data.size();i++) {
            for(int j=0;j<data.values()[i]->queue.size();j++) {
                data.values()[i]->removeFromQueue(b);
            }
        }
    }
public:

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

    static QList< QPair<QObject*, QString> > ss_futureConnetions;

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
    QHash<QObject*, int*> s_priority;

    int getTruePriority() {
        int tp=0;
        for(int i=0;i<s_priority.size();i++) {
            if(s_priority.values()[i]) {
                tp=qMax(*s_priority.values()[i],tp);
            }
        }
        return tp;
    }

public:
    static HCachedPixmap* get(const QUrl& url) {
        if(!s_map.contains(url)) s_map[url]=new HCachedPixmap(url);
        return s_map[url];
    }

public slots:
    int** send(QObject*o,QString m) {
        queue.push_back(qMakePair(o,m)); if(!pix.isNull()) processDownload_2();
        if(!s_priority.contains(o)) s_priority[o]=new int(0);
        return &s_priority[o];
    }
    void download();
    void processDownload(bool err=0); /* internal */
    void processDownload_2();
};

#endif // HFUTURE_H
