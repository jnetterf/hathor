/****************************************************
hbrowser.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#include <QtOAuth>

#ifndef EMBEDDEDBROWSER_H
#define EMBEDDEDBROWSER_H

#define RDIO_CONSUMER_KEY "6r8zwgkfzy2wvyjfdhxgg5yk"
#define RDIO_CONSUMER_SECRET "cGJtmT9Ejw"

#include <QtGui/QWidget>
#include <QtWebKit/QWebView>
#include <QLabel>
#include <QStringList>
#include <QVariant>
#include <QMutex>
#include "libhathor_global.h"
#include "hnotifier.h"

class LIBHATHORSHARED_EXPORT HWebPage : public QWebPage
{
    Q_OBJECT
public:
    HWebPage(QObject* parent=0) : QWebPage(parent) {}
    void javaScriptAlert(QWebFrame *, const QString &msg) {
        emit alert(msg);
    }

signals:
    void alert(const QString& msg);
};

class LIBHATHORSHARED_EXPORT HBrowser : public QWidget
{
    friend class HRdioProvider;
    friend class HRdioLoginAction;
    Q_OBJECT

    static HRunOnceNotifier* s_notifier;

    QWebView* s_webView;
    QList<QWebElement> elements;
    QString s_url;

    QStringList getInputs();
public:
    HBrowser(QWidget *parent = 0);
    ~HBrowser();

    void setHtml(const QString& html,const QUrl&baseUrl=QUrl()) { s_webView->setHtml(html,baseUrl); }

    QString html() const;
    bool htmlContains(const QString& contains) const;
    QString url() const { return s_url; }

protected:
    void hideEvent(QHideEvent *)
    {
        emit visiblityChanged(0);
    }

    void showEvent(QShowEvent *)
    {
        emit visiblityChanged(1);
    }

public slots:
    QVariant doJS(const QString& js);

    void loadPage(QString);
    void loadPageHTTPS(QString);

    //////////////////////////////////
    void loadFinishedLogic(bool s);
    void urlChangedLogic(QUrl url);

    void setInput(QString input,QString value);
    void clickInput(QString input);
signals:
    void newUrl();
    void ready();
    void visiblityChanged(bool);
    void rdioFail();

public:
    static QMultiMap<QByteArray,QByteArray> request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params);
    // DANGER : WILL NOT RETURN UNTIL AN ANSWER HAS BEEN OBTAINED
    static QMultiMap<QByteArray,QByteArray> request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params,const QByteArray &token,const QByteArray &tokenShared);
    // DANGER : WILL NOT RETURN UNTIL AN ANSWER HAS BEEN OBTAINED
};

#endif // EMBEDDEDBROWSER_H
