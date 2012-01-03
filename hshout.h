#ifndef HSHOUT_H
#define HSHOUT_H

#include <QString>
#include "huser.h"

class HShout
{
    QString s_shout;
    HUser& s_shouter;
    QString s_date;
public:
    HShout(QString shout,HUser& shouter,QString date);

    QString getShout() { return s_shout; }
    HUser& getShouter() { return s_shouter; }
    QString getDate() { return s_date; }
};

#endif // HSHOUT_H
