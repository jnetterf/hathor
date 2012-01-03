#ifndef LASTFMEXT_H
#define LASTFMEXT_H

#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkReply>

void lastfmext_autograph( QMap<QString, QString>& params );
void lastfmext_sign( QMap<QString, QString>& params );
QNetworkReply* lastfmext_post( QMap<QString, QString> params );

#endif // LASTFMEXT_H
