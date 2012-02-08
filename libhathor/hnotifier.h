#ifndef HNOTIFIER_H
#define HNOTIFIER_H

#include <QObject>
#include "libhathor_global.h"

class LIBHATHORSHARED_EXPORT HRunOnceNotifier : public QObject {
    Q_OBJECT
public slots:
    void emitNotify() {
        emit notify();
        deleteLater();
    }

signals:
    void notify();
};

#endif // HNOTIFIER_H
