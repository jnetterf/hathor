#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:05:21
#
#-------------------------------------------------

QT       += core phonon xml network

TARGET = rdio-plugin-for-hathor
TEMPLATE = lib
CONFIG += plugin oauth

win32:LIBS += -llastfm0
debug:win32:DESTDIR = ../hathor/debug/plugins
release:win32:DESTDIR = ../hathor/release/plugins
!win32:DESTDIR = ../hathor/plugins/
debug:win32:LIBS += "../libhathor/debug/hathor.lib"
release:win32:LIBS += "../libhathor/release/hathor.lib"

SOURCES += hrdioprovider.cpp \
    hauthaction.cpp

HEADERS += hrdioprovider.h \
    hauthaction.h

INCLUDEPATH+="../libhathor"
