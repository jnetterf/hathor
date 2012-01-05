#include "lastfmext.h"
#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <QCoreApplication>
#include <QStringList>

static inline QString lastfmext_host()
{
    QStringList const args = QCoreApplication::arguments();
    if (args.contains( "--debug"))
        return "ws.staging.audioscrobbler.com";

    int const n = args.indexOf( "--host" );
    if (n != -1 && args.count() > n+1)
        return args[n+1];

    return LASTFM_WS_HOSTNAME;
}

static QUrl lastfmext_url()
{
    QUrl url;
    url.setScheme( "http" );
    url.setHost( lastfmext_host() );
    url.setPath( "/2.0/" );
    return url;
}



void lastfmext_autograph( QMap<QString, QString>& params )
{
    params["api_key"] = lastfm::ws::ApiKey;
//    params["lang"] = iso639();    <-- the whole point of this exercise
}

void lastfmext_sign( QMap<QString, QString>& params )
{
    lastfmext_autograph( params );
    // it's allowed for sk to be null if we this is an auth call for instance
    if (lastfm::ws::SessionKey.size())
        params["sk"] = lastfm::ws::SessionKey;

    QString s;
    QMapIterator<QString, QString> i( params );
    while (i.hasNext()) {
        i.next();
        s += i.key() + i.value();
    }
    s += lastfm::ws::SharedSecret;

    params["api_sig"] = lastfm::md5( s.toUtf8() );
}


QNetworkReply* lastfmext_post( QMap<QString, QString> params )
{
    qDebug()<<"Umm...";
    lastfmext_sign( params );
    QByteArray query;
    QMapIterator<QString, QString> i( params );
    while (i.hasNext()) {
        i.next();
        query += QUrl::toPercentEncoding( i.key() )
               + '='
               + QUrl::toPercentEncoding( i.value() )
               + '&';
    }

//    ContentTypeHeader
    qDebug()<<"Ready...";
    QNetworkRequest nr(lastfmext_url());
    nr.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    qDebug()<<"And...";
    return lastfm::nam()->post( nr, query );
}
