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

/*
The blinking | isnt there when you enter your name/password
what the heck is that stupid plaid arrow?
make a back button for your player|< >|
make it so you can hide the top artists and your recommendations on the top of the artist screen
make a scroll wheel for the artist page
make the play/plus bottom right triangles better/bigger area
make an expand album artwork thing
a timebar for tracks
hover over for what it does
make a home button (think android) at any place
backspace = back
Home page.hover over. try to get to link, fail
be able to see the homepage offline?
how to log out?
grooveshark
*/

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
