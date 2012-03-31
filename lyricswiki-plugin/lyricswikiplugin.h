#ifndef LYRICSWIKIPLUGIN_H
#define LYRICSWIKIPLUGIN_H

#include "lyricswiki-plugin_global.h"
#include "hplugin.h"
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class LYRICSWIKIPLUGINSHARED_EXPORT HSendLyricsAction : public QObject{
    Q_OBJECT
    HTrack* s_t;
    QObject* s_obj;
    QString s_member;
public:
    HSendLyricsAction(HTrack* t,QObject* obj,QString member) : s_t(t), s_obj(obj), s_member(member) {
        QString l=t->getTrackName().toLower();
        bool f=1;
        for(int i=0;i<l.size();i++) {
            if(f) l[i]=l[i].toUpper();
            if(l[i]==' ') f=1;
            else f=0;
        }

        QNetworkRequest nr("http://lyrics.wikia.com/api.php?action=query&prop=revisions&rvprop=content&format=json&titles="+t->getArtistName()+":"+l);
        nr.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded; charset=utf-8");
        QNetworkAccessManager* manager=new QNetworkAccessManager;
        manager->get(nr);
        connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(procReply(QNetworkReply*)));
    }
public slots:
    void procReply(QNetworkReply* reply) {
        QString b=reply->readAll();
        if(!b.contains("[{\"*\":\"{")) {
            QMetaObject::invokeMethod(s_obj,s_member.toAscii().data(),Qt::QueuedConnection,Q_ARG(QString,"No lyrics available."));
        } else {
            b.remove(0,b.indexOf("[{\"*\":\"")+7);
            while(b.contains('|')) b.remove(b.indexOf('|'),1);
            while(b.indexOf('{')<b.indexOf('}')) {
                int l=b.indexOf('{');
                b.remove(b.indexOf('{'),b.indexOf('}')-b.indexOf('{'));
                while((b.indexOf('}')<b.indexOf('{'))||(b.contains('}')&&!b.contains('{'))) {
                    b.remove(l,b.indexOf('}')+1-l);
                }
            }
            b.replace("\\n","\n");
            b.replace("\\\"","\"");
            b.replace("\\r","");
            while(b.contains("\\u")) {
                int i=b.indexOf("\\u");
                b.remove(i,2);
                b[i]=b.mid(i,4).toInt(0,16);
                b.remove(i+1,3);

            }
            b.remove("<lyrics>");
            b.remove("<\\/lyrics>");
            while(b.startsWith('\n')||b.startsWith(' ')) b.remove(0,1);
            while(b.endsWith('\n')) b.chop(1);
            QMetaObject::invokeMethod(s_obj,s_member.toAscii(),Qt::QueuedConnection,Q_ARG(QString,QString(b)));
        }
        deleteLater();
    }
};

class HLWikiPlugin : public QObject, public HPlugin
{
    Q_OBJECT
    Q_INTERFACES(HPlugin)
    friend class HLocalIntro;
    friend class HLocalProvider;
    QSet<QString> s;
public:
    HLWikiPlugin() {s.insert("getLyrics");}
    virtual HAbstractTrackProvider* trackProvider() { return 0; }
    virtual QWidget* initWidget() { return 0;}
    virtual QString name() { return "Lyric Wiki"; }
    virtual QSet<QString> abilities() { return s; }
    virtual int** send(const QString & com, HObject *track, QObject *send, QString member, QObject *) {
        if(com=="getLyrics") {
            HTrack* t=dynamic_cast<HTrack*>(track);
            if(t) {
                HSendLyricsAction* a=new HSendLyricsAction(t,send,member);
                connect(send,SIGNAL(destroyed()),a,SLOT(deleteLater()));
            }
        }
        return 0;
    }
    virtual int getScore(const QString &com, HObject *, QObject *, QString, QObject *) { return (com=="getLyrics")?99:-1; }
    bool isLocal(const QString &, HObject *, QObject *, QString , QObject *) { return 0; }

    virtual QWidget* configurationWidget() { return 0; }
};

#endif // LYRICSWIKIPLUGIN_H
