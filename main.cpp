#include "hmainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QStyle>
#include <QIcon>
#include <QFile>
#include <QTime>

int main(int argc, char** argv) {
    QApplication app(argc,argv);
    app.setWindowIcon(QIcon(":/icons/hathor-logo.png"));

    qsrand(QTime::currentTime().msec());

    app.setStyle("plastique");
    QFile stylefile(":/icons/styleSheet.txt");
    stylefile.open(QIODevice::ReadOnly);
    app.setStyleSheet(QString(stylefile.readAll()));

    new HMainWindow();
    app.exec();
}
