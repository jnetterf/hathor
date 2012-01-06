#ifndef HOBJECT_H
#define HOBJECT_H

#include <QObject>

class HObject : public QObject  //just in case...
{
public:
    HObject(QObject* parent=0);
};

#endif // HOBJECT_H
