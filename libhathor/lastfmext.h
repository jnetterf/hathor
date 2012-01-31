#ifndef LASTFMEXT_H
#define LASTFMEXT_H

#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkReply>
#include "libhathor_global.h"

LIBHATHORSHARED_EXPORT void lastfmext_autograph( QMap<QString, QString>& params );
LIBHATHORSHARED_EXPORT void lastfmext_sign( QMap<QString, QString>& params );
LIBHATHORSHARED_EXPORT QNetworkReply* lastfmext_post( QMap<QString, QString> params );

#endif // LASTFMEXT_H
