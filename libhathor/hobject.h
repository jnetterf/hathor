#ifndef HOBJECT_H
#define HOBJECT_H

#include <QObject>
#include "libhathor_global.h"

class LIBHATHORSHARED_EXPORT HObject : public QObject  //just in case...
{
public:
    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    static QString eliminateHtml(const QString& o);
    HObject(QObject* parent=0);
};

#endif // HOBJECT_H
