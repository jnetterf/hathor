#include <QNetworkRequest>
#include <QSettings>
#include <QObject>
#include "libhathor_global.h"

#ifndef HNETTLOGER_H
#define HNETTLOGER_H

class LIBHATHORSHARED_EXPORT HNettLogger : public QObject
{
    Q_OBJECT
    HNettLogger();
    static HNettLogger* s_singleton;
    QSettings s_log;
    bool s_active;
public:
    static HNettLogger* singleton() { if(!s_singleton) s_singleton=new HNettLogger; return s_singleton; }
public slots:
    void log(QString);
    void pushLog();
    void handleReply();
};

inline void HL(QString i) { HNettLogger::singleton()->log(i); }

#endif // HNETTLOGER_H
