/****************************************************
hbrowser.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#ifndef EMBEDDEDBROWSER_H
#define EMBEDDEDBROWSER_H

#define RDIO_CONSUMER_KEY "6r8zwgkfzy2wvyjfdhxgg5yk"
#define RDIO_CONSUMER_SECRET "cGJtmT9Ejw"

#include <QtGui/QWidget>
#include <QtWebKit/QWebView>
#include <QLabel>
#include <QStringList>
#include <QVariant>

class HWebPage : public QWebPage
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

class HBrowser : public QWidget
{
    friend class HRdioInterface;
    Q_OBJECT

    QWebView* s_webView;
    QList<QWebElement> elements;

    QStringList getInputs();
public:
    HBrowser(QWidget *parent = 0);
    ~HBrowser();

    void setHtml(const QString& html,const QUrl&baseUrl=QUrl()) { s_webView->setHtml(html,baseUrl); }

    QString html() const;
    bool htmlContains(const QString& contains) const;

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

    //////////////////////////////////
    void loadFinishedLogic(bool s);
    void urlChangedLogic(QUrl url);

    void setInput(QString input,QString value);
    void clickInput(QString input);
signals:
    void ready();
    void visiblityChanged(bool);

public:
    static QMultiMap<QByteArray,QByteArray> request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params);
    static QMultiMap<QByteArray,QByteArray> request(const QByteArray& consumerKey,const QByteArray& consumerSecret,const QString& url,QMultiMap<QByteArray,QByteArray> params,const QByteArray &token,const QByteArray &tokenShared);
};

#endif // EMBEDDEDBROWSER_H
