#include "hbrowser.h"
#include "hauthaction.h"
#include "hloginwidget.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QStyle>

int main(int argc, char** argv) {
    QApplication app(argc,argv);
    HLoginWidget a;
    a.show();
    app.exec();
}
