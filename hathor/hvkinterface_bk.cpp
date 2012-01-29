#include "hvkinterface.h"
#include <QDebug>
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>
#include <lastfm.h>

#define VK_API_ID "2757104"
#define VK_API_SECRET "Rzx58ThxNxsbeej2WvJZ"

//QString vk_md5(QMap<QString, QString>& params) {
//    QString s;
//    QMapIterator<QString, QString> i( params );
//    s+=HVkInterface::singleton()->s_user;
//    while (i.hasNext()) {
//        i.next();
////        if(i.key()=="v") continue;
////        if(i.key()=="format") continue;
////        if(i.key()=="callback") continue;
//        s += i.key() + '=' + i.value();
//    }
//    s += VK_API_SECRET;
//    qDebug()<<s;

//    params["sig"] = lastfm::md5( s.toUtf8() );
//}

//QNetworkReply* vk_post( QMap<QString, QString> params )
//{
//    vk_md5(params);
//    QByteArray query;
//    QMapIterator<QString, QString> i( params );
//    while (i.hasNext()) {
//        i.next();
//        query += QUrl::toPercentEncoding( i.key() )
//               + '='
//               + QUrl::toPercentEncoding( i.value() )
//               + '&';
//    }
//    if(query.endsWith('&')) query.chop(1);

//    QNetworkRequest nr(QUrl("http://api.vkontakte.ru/api.php"));
//    nr.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

//    qDebug()<<query;
//    return lastfm::nam()->post( nr, query );
//}


HVkProvider* HVkProvider::s_singleton=0;

HVkProvider::HVkProvider() {
    s_browser.loadPage(QString("http://oauth.vkontakte.ru/oauth/authorize?client_id=")+QString(VK_API_ID)+QString("&scope=audio,offline&redirect_uri=http://api.vkontakte.ru/blank.html&response_type=code"));
    connect(&s_browser,SIGNAL(ready()),this,SLOT(checkBrowser()));
}

void HVkProvider::checkBrowser() {
    qDebug()<<s_browser.url();
    if(s_browser.url().contains("&scope=audio,offline")||s_browser.url().contains("state=&display=page&m=4")) {
        //TRANSLATE
        s_browser.doJS("document.getElementsByClassName(\"label\")[0].innerHTML=\"Telephone or e-mail\"");
        s_browser.doJS("document.getElementsByClassName(\"label\")[1].innerHTML=\"Password:\"");
        s_browser.doJS("document.getElementById(\"expire_checkbox\").innerHTML=\"<div></div><font><font class>Don't remember me</font></font>\"");
        s_browser.doJS("document.getElementsByClassName(\"oauth_header\")[0].innerHTML=\"<font><font class>Log in</font></font>\"");
        s_browser.doJS("document.getElementById(\"install_allow\").innerHTML=\"<font><font class>Log in</font></font>\"");
        s_browser.doJS("document.getElementById(\"install_cancel\").innerHTML=\"<font><font class>Cancel</font></font>\"");
        s_browser.show();
    } else if(s_browser.url().contains("error")) {
        if(QMessageBox::critical(0,"Error getting access","Could not get access to Vk. Try again?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes) {
            s_browser.loadPage("http://oauth.vkontakte.ru/oauth/authorize?client_id=2757104&scope=audio,offline&redirect_uri=http://api.vkontakte.ru/blank.html&response_type=token");
        }
    } else if(s_browser.url().contains("code")&&s_code.isEmpty()) {
        s_browser.hide();
        s_code=s_browser.url();
        s_code.remove(0,s_code.indexOf("code=")+5);
        qDebug()<<s_code;
        s_browser.loadPageHTTPS("https://api.vkontakte.ru/oauth/access_token?client_id="+QString(VK_API_ID)+"&client_secret="+VK_API_SECRET+"&code="+s_code);
        QMap<QByteArray,QByteArray> map;
        map.insert("client_id",VK_API_ID);
        map.insert("client_secret",VK_API_SECRET);
        map.insert("code",s_code.toAscii());
        map.insert("format","xml");

        QMultiMap<QByteArray,QByteArray> a=HBrowser::request(VK_API_ID,VK_API_SECRET,"https://api.vkontakte.ru/oauth/access_token",map);
        if(a.size()) {
            s_accessToken=a.values()[0];
            s_accessToken.remove(0,s_accessToken.indexOf("n\":\"")+4);
            s_accessToken.truncate(s_accessToken.indexOf("\""));
            s_userId=a.values()[0];
            s_userId.remove(0,s_userId.indexOf("user_id\":")+9);
            s_userId.truncate(s_userId.indexOf("}"));
            qDebug()<<s_userId<<s_accessToken;
        }
        map.clear();
        map.insert("uuid","id159869669");
        map.insert("access_token",s_accessToken.toUtf8());
        a=HBrowser::request(VK_API_ID,VK_API_SECRET,"https://api.vkontakte.ru/method/getProfiles.xml",map);
        qDebug()<<a;

    }
}

QString HVkProvider::search(QString query, QString artistF, QString trackF) {

}
