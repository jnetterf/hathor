#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:05:21
#
#-------------------------------------------------

QT       += core phonon xml network

TARGET = local-plugin-for-hathor
TEMPLATE = lib
CONFIG += plugin oauth

DESTDIR = /usr/lib/hathor/plugins

SOURCES += hlocalprovider.cpp

HEADERS += hlocalprovider.h

INCLUDEPATH+="../libhathor"

unix:!symbian {
    target.path = /usr/share/hathor-20120128/plugins
    INSTALLS += target
}
