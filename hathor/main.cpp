#include "hmainwindow.h"
#include "habstractmusicinterface.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QStyle>
#include <QIcon>
#include <QFile>
#include <QTime>

#include "htrack.h"

int main(int argc, char** argv) {
    qRegisterMetaType< QList<HTrack*> >("QList<HTrack*>");
    qRegisterMetaType< HTrack* >("HTrack*");
    qRegisterMetaType< QList<HTag*> >("QList<HTag*>");
    qRegisterMetaType< QList<HShout*> >("QList<HShout*>");
    qRegisterMetaType< HShout* >("HShout*");
    qRegisterMetaType< QList<HAlbum*> >("QList<HAlbum*>");
    qRegisterMetaType< HAlbum* >("HAlbum*");
    qRegisterMetaType< QList<HArtist*> >("QList<HArtist*>");
    qRegisterMetaType< HArtist* >("HArtist*");
    qRegisterMetaType< HAbstractTrackInterface* >("HAbstractTrackInterface*");
    qRegisterMetaType< HAbstractTrackProvider* >("HAbstractTrackProvider*");

    QApplication app(argc,argv);
    app.setWindowIcon(QIcon(":/icons/hathor-logo.png"));

    qsrand(QTime::currentTime().msec());

    app.setStyle("plastique");
    QFile stylefile(":/icons/styleSheet.txt");
    stylefile.open(QIODevice::ReadOnly);
    app.setStyleSheet(QString(stylefile.readAll()));

    HPlayer::singleton();

    new HMainWindow();
    app.exec();
}
