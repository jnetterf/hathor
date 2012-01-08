#ifndef HNOTIFIER_H
#define HNOTIFIER_H

#include <QObject>

class HRunOnceNotifier : public QObject {
    Q_OBJECT
public slots:
    void emitNotify() {
        emit notify();
        deleteLater();
    }

signals:
    void notify();
};

#define H_BEGIN_RUN_ONCE_MINIMAL \
    if(getting) { \
    QEventLoop ev; \
    connect(getting,SIGNAL(notify()),&ev,SLOT(quit())); \
    ev.exec(); \
    return; \
    } \
    getting=new HRunOnceNotifier;

#define H_BEGIN_RUN_ONCE \
    H_BEGIN_RUN_ONCE_MINIMAL \
    Q_ASSERT(!got); \
    if(got) { \
    return; \
    } \
    got=1;

#define H_END_RUN_ONCE_MINIMAL \
    getting->emitNotify(); \
    getting=0; \

#define H_END_RUN_ONCE \
    H_END_RUN_ONCE_MINIMAL \
    got=1;

#endif // HNOTIFIER_H
