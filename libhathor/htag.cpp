#include "htag.h"

#include "hartist.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QEventLoop>
#include <QSettings>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"

QHash<QString, HTag*> HTag::_map;

HTag::HTag(QString tag) : s_tag(tag)
{
}

HTag& HTag::get(QString tag) {
    if(_map.value(tag,0)) {
        return *_map.value(tag);
    }

    _map.insert(tag,new HTag(tag));
    return get(tag);
}

int HTag::getReach() {
    if(s_infoData.got) {
        return s_infoData.reach;
    }
    s_infoData.getData(s_tag);
    return getReach();
}

int HTag::getTaggings() {
    if(s_infoData.got) {
        return s_infoData.taggings;
    }
    s_infoData.getData(s_tag);
    return getTaggings();
}

QString HTag::getSummary() {
    if(s_infoData.got) {
        return s_infoData.summary;
    }
    s_infoData.getData(s_tag);
    return getSummary();
}

QString HTag::getContent() {
    if(s_infoData.got) {
        return s_infoData.content;
    }
    s_infoData.getData(s_tag);
    return getContent();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void HTag::InfoData::getData(QString tag) {
    if(got) {
        return;
    }
    got=1;

    /*
        QStringList tags;
        QString summary;
        QString content;
        int listenerCount;
        int playCount;
        int userPlayCount;
        bool loved;
        bool got;
        */
    QSettings sett("Nettek","Hathor_tagInfo");
    if(sett.value("cache for "+tag,0).toInt()==2) {
        summary=sett.value("summary for "+tag).toString();
        content=sett.value("content for "+tag).toString();
        reach=sett.value("reach for "+tag).toInt();
        taggings=sett.value("taggings for "+tag).toInt();
        return;
    }

    QMap<QString, QString> params;
    params["method"] = "tag.getInfo";
    params["tag"] = tag;
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    if(!reply->isFinished()||reply->error()!=QNetworkReply::NoError) {
        qDebug()<<reply->errorString();
        got=0;
        QEventLoop loop; QTimer::singleShot(2850,&loop,SLOT(quit())); loop.exec();
        getData(tag);
        return;
    }

    taggings=0;
    reach=0;
    summary="";
    content="";
    try {
        QDomDocument doc;
        doc.setContent( QString::fromUtf8(reply->readAll().data()) );

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "reach") reach = l.toText().data().toInt();
                    else if ( m.nodeName() == "taggings") taggings = l.toText().data().toInt();

                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) summary = k.toText().data();
                        else if ( l.nodeName() == "content" ) content = HObject::eliminateHtml(k.toText().data());
                    }
                }
            }
        }

    } catch (std::runtime_error& e) {
        qWarning() << e.what();
    }
    sett.setValue("cache for "+tag,2);
    sett.setValue("reach for "+tag,reach);
    sett.setValue("taggings for "+tag,taggings);
    sett.setValue("summary for "+tag,summary);
    sett.setValue("content for "+tag,content);
}
