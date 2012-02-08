#include "hnettloger.h"
#include "lastfmext.h"
#include <lastfm/NetworkAccessManager>
#include <lastfm/misc.h>
#include <lastfm/ws.h>
#include <QNetworkReply>
#include <QTimer>

HNettLogger::HNettLogger() : s_log("Nettek","logging"), s_active(0) {}

HNettLogger* HNettLogger::s_singleton=0;

void HNettLogger::log(QString s) {
    //        s_l.setValue("permission","yes");
    s=QTime::currentTime().toString("hhmmsszzz::")+s;
    QStringList log=s_log.value("log",QStringList()).toStringList();
    log.push_back(s);
    s_log.setValue("log",s);
    if((s_log.value("permission")=="yes"||s_log.value("permission")=="importantOnly")&&!s_active) {
        pushLog();
    }
}

void HNettLogger::pushLog() {
    QStringList log=s_log.value("log",QStringList()).toStringList();
    if(!log.size()) return;
    if(s_active) return;
    s_active=1;

    QNetworkRequest nr(QUrl("http://nettek.ca/cgi-bin/logserver.perl"));
    QByteArray query="user="+lastfm::ws::Username.toAscii()+"&log="+log.front().toAscii();
    nr.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QNetworkReply* reply = lastfm::nam()->post( nr, query );
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(handleReply()));
}

void HNettLogger::handleReply() {
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    if(!reply) return;
    if(reply->error()==QNetworkReply::NoError) {
        QStringList log=s_log.value("log",QStringList()).toStringList();
        log.pop_front();
        s_log.setValue("log",log);
    }
    s_active=0;
    QTimer::singleShot(0,this,SLOT(pushLog()));
}
