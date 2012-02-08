#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "hbackground.h"
#include "hgraphicsview.h"
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>
#include <QXmlReader>
#include <QSettings>
#include "hartist.h"
#include <lastfm/ws.h>
#include <QApplication>
#include <QMutex>
#include "lastfmext.h"

namespace Ui {
class HLoginWidget;
}

class FadePixmap : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal echoOpacity READ echoOpacity WRITE setEchoOpacity)
public slots:
    void setEchoOpacity(qreal opacity){QGraphicsPixmapItem::setOpacity(opacity);}
    void show() { QGraphicsPixmapItem::show(); }
public:
    qreal echoOpacity() const { return QGraphicsPixmapItem::opacity();}
};

class MagicLineEdit : public QLineEdit {
    Q_OBJECT public:
    MagicLineEdit(QWidget* parent=0) : QLineEdit(parent) {
        setStyleSheet("QLineEdit {"
                      "border: 1px solid #141414;"
                      "border-radius: 4px;"
                      "color: #1e1e1e;"
                      "font-family: Arial;"
                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                       "stop: 0 #ffffff, stop: 1 #aaaaaa);"
                      "selection-background-color: #e8e8e8;"
                      "selection-color: #262627"
                      "}");
    }
    MagicLineEdit(const QString &a, QWidget *parent) : QLineEdit(a,parent) {}
    void focusInEvent(QFocusEvent *) { emit gotFocus(); }
signals:
    void gotFocus();
};

class HLoginWidget : public HGraphicsView {
    Q_OBJECT
public:
    explicit HLoginWidget(QWidget *parent = 0);

public slots:
    // LOGIN
    void continueLoading();
    void showTabHint();
    void doPassword();
    void doLogin();
    void doLogin2();

    void finish(int a=0);

    void openLink(QString);

signals:
    void showMainContext();

private:
    bool stage1;
    FadePixmap* px;
    QGraphicsScene* sc;
    QGraphicsTextItem* tx;
    QLabel* affil, * nothanks;
    QPropertyAnimation* anim,*anim2,*anim3;
    MagicLineEdit*a,*b;
    Ui::HLoginWidget *ui;
};

#endif // MAINWINDOW_H
