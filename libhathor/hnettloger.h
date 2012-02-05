#include <QEventLoop>
#include <QNetworkRequest>

#ifndef HNETTLOGER_H
#define HNETTLOGER_H

class HNettLoger
{
    HNettLoger();
    static HNettLoger* s_singleton;
public:
    static HNettLoger* singleton() { if(!s_singleton) s_singleton=new HNettLoger; return s_singleton; }
    void log(QString);
};

inline void HL(QString i) { HNettLoger::singleton()->log(i); }

#endif // HNETTLOGER_H
