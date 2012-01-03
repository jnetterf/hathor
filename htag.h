#ifndef HTAG_H
#define HTAG_H

#include <QStringList>
#include <QPixmap>

class HTag
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
    static QMap<QString, HTag*> _map;
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
