#include "hobject.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QRegExp>

QString HObject::eliminateHtml(const QString &o) {
    QString r=o;
    r.replace("<br>","\n");
    r.replace((char)13,"\n");
    r.replace((char)15,"\n");
    r.replace((char)11,"");
    r.replace((char)10,"\n");
    r.replace("&quot;","\"");
    r.replace("&ndash;","-");
    r.replace("&mdash;","-");
    r.replace("&amp;","&");
    while(r.contains("\n ")) r.replace("\n ","\n");
    while(r.contains(" \n")) r.replace(" \n","\n");
    r.remove(QRegExp("<[^>]*>"));

    return r;
}

HObject::HObject(QObject *parent) : QObject(parent)
{
}
