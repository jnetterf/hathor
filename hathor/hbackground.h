#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QDebug>
#include "hartist.h"

class ArtistAvatar;

class HBackground : public QObject {
    Q_OBJECT
public:
    HBackground(QGraphicsScene*sc);
public slots:
    void showContext();
    void showStuff();
private:
    QGraphicsScene* _sc;
signals:
    void showContext(HArtist&);
};

#endif // BACKGROUND_H
