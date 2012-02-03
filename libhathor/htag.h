#ifndef HTAG_H
#define HTAG_H

#include "hobject.h"
#include "libhathor_global.h"

#include <QStringList>
#include <QPixmap>

class LIBHATHORSHARED_EXPORT HTag : public HObject
{
    QString s_tag;
public:
    static HTag& get(QString tag);

    QString getTagName() { return s_tag; }

    int getReach();
    int getTaggings();
    QString getSummary();
    QString getContent();

private:
    static QHash<QString, HTag*> _map;
    HTag(QString tag);  // use HTag::get(name)

    struct InfoData {
        QString summary;
        QString content;
        int reach;
        int taggings;
        bool got;
        InfoData() : got(0) {}

        void getData(QString tag);
    } s_infoData;

private:
    //Degenerate copy and assignment
    HTag(const HTag&);
    HTag& operator=(const HTag&);
};

#endif // HTAG_H
