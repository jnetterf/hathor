#include "hnettloger.h"
#include "lastfmext.h"
#include <lastfm/NetworkAccessManager>
#include <lastfm/misc.h>
#include <lastfm/ws.h>
#include <QNetworkReply>

HNettLoger::HNettLoger() {}

HNettLoger* HNettLoger::s_singleton=0;

void HNettLoger::log(QString s) {
    s=QTime::currentTime().toString("hh mm ss zzz::")+s;
    QNetworkRequest nr(QUrl("http://nettek.ca/cgi-bin/logserver.perl"));
    QByteArray query="user="+lastfm::ws::Username.toAscii()+"&log="+s.toAscii();
    nr.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QNetworkReply* reply = lastfm::nam()->post( nr, query );
    QObject::connect(reply,SIGNAL(finished()),reply,SLOT(deleteLater()));
}
