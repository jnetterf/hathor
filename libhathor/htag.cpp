#include "htag.h"

#include "hartist.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <QHttp>
#include <QSettings>
#include <QDomDocument>
#include <lastfm/ws.h>
#include "lastfmext.h"

QHash<QString, HTag*> HTag::_map;

HTag::HTag(QString tag) : s_tag(tag), s_infoData(tag)
{
}

HTag& HTag::get(QString tag) {
    if(_map.value(tag,0)) {
        return *_map.value(tag);
    }

    _map.insert(tag,new HTag(tag));
    return get(tag);
}

void HTag::sendReach(QObject *o, QString m) {
    s_infoData.sendProperty("reach",o,m);
}

void HTag::sendTaggings(QObject *o, QString m) {
    s_infoData.sendProperty("taggings",o,m);
}

void HTag::sendSummary(QObject *o, QString m) {
    s_infoData.sendProperty("summary",o,m);
}

void HTag::sendContent(QObject *o, QString m) {
    s_infoData.sendProperty("content",o,m);
}

//////////////////////////////////////////////////////////////////////////////////////////////

HTag::InfoData::InfoData(QString tag) {
    QMap<QString, QString> params;
    params["method"] = "tag.getInfo";
    params["tag"] = tag;
    setParams(params);

    QByteArray b=QCryptographicHash::hash(tag.toUtf8()+"TAGDATA",QCryptographicHash::Md5).toHex();
    addProperty<int>("reach",b);
    addProperty<int>("taggings",b);
    addProperty<QString>("summary",b);
    addProperty<QString>("content",b);
}

bool HTag::InfoData::process(const QString &data) {
    try {
        QDomDocument doc;
        doc.setContent( data );
        QStringList tags;

        QDomElement element = doc.documentElement();

        for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            for (QDomNode m = n.firstChild(); !m.isNull(); m = m.nextSibling()) {
                for (QDomNode l = m.firstChild(); !l.isNull(); l = l.nextSibling()) {
                    if ( m.nodeName() == "reach") setProperty("reach",l.toText().data().toInt());
                    else if ( m.nodeName() == "taggings") setProperty("taggings",l.toText().data().toInt());

                    for (QDomNode k = l.firstChild(); !k.isNull(); k = k.nextSibling()) {
                        if ( l.nodeName() == "summary" ) setProperty("summary",k.toText().data());
                        else if ( l.nodeName() == "content" ) setProperty("content",HObject::eliminateHtml(k.toText().data()));
                    }
                }
            }
        }
    } catch (std::runtime_error& e) {
        qWarning() << e.what();
        return 0;
    }
    return 1;
}
