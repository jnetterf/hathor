/****************************************************
hauthaction.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#ifndef ELOGINACTION_H
#define ELOGINACTION_H

#include "haction.h"
#include <QByteArray>

class HAuthAction : public HAction
{
    Q_OBJECT
    QString s_username;
    QString s_password;
    QByteArray s_token;
    QByteArray s_tokenSecret;
public:
    HAuthAction(HBrowser&browser,QString username,QString password);
public slots:
    void continue1();
    void continue2();
    void continue3();

signals:
    void gotOauth(QByteArray token, QByteArray tokenSecret, QByteArray oauthToken, QByteArray oauthTokenSecret);
};

#endif // ELOGINACTION_H
