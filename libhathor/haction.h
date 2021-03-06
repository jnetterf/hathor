/****************************************************
haction.h

                    Part of Hathor
        Copyright (C) Joshua Netterfield 2011

                 All rights reserved.
*****************************************************/

#ifndef HACTION_H
#define HACTION_H

#include <QObject>
#include "libhathor_global.h"
#include "hbrowser.h"

class LIBHATHORSHARED_EXPORT HAction : public QObject
{
    Q_OBJECT
protected:
    HBrowser& s_browser;
public:
    HAction(HBrowser& browser);
    virtual void init() {}
signals:
    void done();
    void error(QString);
    void info(QString);
};

#endif // HACTION_H
