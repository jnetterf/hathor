#include "hbrowser.h"
#include "hauthaction.h"
#include "hloginwidget.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QStyle>

int main(int argc, char** argv) {
    QApplication app(argc,argv);

    app.setStyle("plastique");
    QFile stylefile(":/icons/styleSheet.txt");
    stylefile.open(QIODevice::ReadOnly);
    app.setStyleSheet(QString(stylefile.readAll()));

    HLoginWidget a;
    a.show();
    app.exec();
}
