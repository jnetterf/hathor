#ifndef HOBJECT_H
#define HOBJECT_H

#include <QObject>
#include "libhathor_global.h"

class LIBHATHORSHARED_EXPORT HObject : public QObject  //just in case...
{
public:
    static QString eliminateHtml(const QString& o);
    HObject(QObject* parent=0);
};

#endif // HOBJECT_H
