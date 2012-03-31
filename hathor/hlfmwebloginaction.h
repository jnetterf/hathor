#ifndef HLFMWEBLOGINACTION_H
#define HLFMWEBLOGINACTION_H

#include "haction.h"
#include <QTimer>
#include <QDebug>
#include <QEventLoop>

class HLfmWebLoginAction : public HAction
{
    Q_OBJECT
    QString s_username, s_password;
public:
    HLfmWebLoginAction(HBrowser& browser, QString username, QString password) : HAction(browser), s_username(username), s_password(password) {
        connect(&s_browser,SIGNAL(ready()),this,SLOT(next()));
        s_browser.loadPage("http://www.last.fm/login");
        QTimer::singleShot(12000,this,SIGNAL(done()));
    }
public slots:
    void next() {   //BUGFIX THIS!
        disconnect(&s_browser,SIGNAL(ready()),this,SLOT(next()));
        connect(&s_browser,SIGNAL(ready()),this,SLOT(last()));
        s_browser.setInput("username",s_username);
        s_browser.setInput("password",s_password);
        s_browser.doJS("document.getElementsByName(\"login\")[0].click()");
    }
    void last() {
        disconnect(&s_browser,SIGNAL(ready()),this,SLOT(last()));
        s_browser.doJS("a=LFM.Join({\"id\":\"1226013\",\"type\":20,\"name\":\"Hathor Users\"}, {parameters: null}).dialog");
        QEventLoop loop;
        QTimer::singleShot(1000,&loop,SLOT(quit()));
        loop.exec();
        s_browser.doJS("a.confirmButton.click()");
        emit done();
    }
};

class HShoutAction : public HAction
{
    Q_OBJECT
    QString s_shout;
public:
    HShoutAction(HBrowser& browser, QString url, QString shout) : HAction(browser), s_shout(shout) {
        connect(&s_browser,SIGNAL(ready()),this,SLOT(next()));
        s_browser.loadPage(url);
        QTimer::singleShot(4000,this,SIGNAL(done()));
    }
public slots:
    void next() {   //BUGFIX THIS
        disconnect(&s_browser,SIGNAL(ready()),this,SLOT(next()));
        connect(&s_browser,SIGNAL(ready()),this,SIGNAL(done()));
        s_browser.setInput("shoutmsg",s_shout);
        s_browser.clickInput("sbPost");
    }
};

class HLfmWebManager : public QObject
{
    Q_OBJECT
    HBrowser s_browser;
    HAction* s_action;
    QList< QPair<QString,QString> > s_Q;
    static HLfmWebManager* s_singleton;
    friend class HLoginWidget;

    HLfmWebManager(QString username,QString password) : s_browser(0,false), s_action(new HLfmWebLoginAction(s_browser,username,password)) {
        s_singleton=this;
        connect(s_action,SIGNAL(done()),this,SLOT(cleanUp()));
    }
public:
    static HLfmWebManager* singleton() { return s_singleton; }

public slots:
    void cleanUp() {
        s_action->deleteLater();
        s_action=0;
        if(s_Q.size()) {
            QPair<QString,QString> l;
            l=s_Q.takeFirst();
            shout(l.first,l.second);
        }
    }
    void shout(QString url,QString shout) {
        if(!s_action) {
            s_action=new HShoutAction(s_browser,url,shout);
        } else {
            s_Q.push_back(qMakePair(url,shout));
        }
    }
};

#endif // HLFMWEBLOGINACTION_H
