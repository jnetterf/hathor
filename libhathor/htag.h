#ifndef HTAG_H
#define HTAG_H

#include "hobject.h"
#include "hfuture.h"
#include "libhathor_global.h"

#include <QStringList>
#include <QPixmap>

class LIBHATHORSHARED_EXPORT HTag : public HObject
{
    QString s_tag;
public:
    static HTag& get(QString tag);

    QString getTagName() { return s_tag; }

    void sendReach(QObject* o,QString m);   /*int*/
    void sendTaggings(QObject* o,QString m); /*int*/
    void sendSummary(QObject* o,QString m); /* QString */
    void sendContent(QObject* o,QString m); /* QString */

private:
    static QHash<QString, HTag*> _map;
    HTag(QString tag);  // use HTag::get(name)

    struct InfoData : HCachedInfo {
        friend class HTag;
        InfoData(QString tag);
        bool process(const QString& data);
    } s_infoData;

private:
    //Degenerate copy and assignment
    HTag(const HTag&);
    HTag& operator=(const HTag&);
};

#endif // HTAG_H
