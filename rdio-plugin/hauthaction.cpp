/****************************************************
eloginaction.cpp

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#include "hauthaction.h"
#include <QSettings>
#include <QDebug>

HAuthAction::HAuthAction(HBrowser &browser, QString username, QString password) : HAction(browser), s_username(username), s_password(password)
{
    QSettings sett("Nettek","rdioKeys");
    sett.setValue("username",username);
    sett.setValue("password",password);
    QMultiMap<QByteArray,QByteArray> map1;
    map1.insert("oauth_callback", "oob");
    QMultiMap<QByteArray,QByteArray> p=HBrowser::request(RDIO_CONSUMER_KEY,RDIO_CONSUMER_SECRET,"http://api.rdio.com/oauth/request_token",map1);

    s_token=p.value("oauth_token");
    s_tokenSecret=p.value("oauth_token_secret");
    qDebug() << s_token << s_tokenSecret;
    qDebug() << p;
    browser.loadPage(QByteArray::fromPercentEncoding(p.value("login_url"))+"?oauth_token="+s_token);
    connect(&browser,SIGNAL(ready()),this,SLOT(continue1()));
//    browser.show();
}

void HAuthAction::continue1()
{
    disconnect(&s_browser,SIGNAL(ready()),this,SLOT(continue1()));
    s_browser.show();
    connect(&s_browser,SIGNAL(ready()),this,SLOT(continue2()));
}

void HAuthAction::continue2()
{
    qDebug() << "!!";
    disconnect(&s_browser,SIGNAL(ready()),this,SLOT(continue2()));
    connect(&s_browser,SIGNAL(ready()),this,SLOT(continue3()));
}

void HAuthAction::continue3()
{
    qDebug() << "..." << s_browser.html();
    return;
    if(!s_browser.htmlContains("pin")) {
    }
    QString html = s_browser.html();
    html.remove(0,html.indexOf("<strong>")+8);
    html.truncate(html.indexOf("</strong>"));
    QMultiMap<QByteArray,QByteArray> p1;
    p1.insert("oauth_verifier",html.toUtf8());
    QMultiMap<QByteArray,QByteArray> p=HBrowser::request(RDIO_CONSUMER_KEY,RDIO_CONSUMER_SECRET,"http://api.rdio.com/oauth/access_token",p1,s_token,s_tokenSecret);

    emit gotOauth(s_token,s_tokenSecret,p.value("oauth_token"),p.value("oauth_token_secret"));
    emit done();
}

