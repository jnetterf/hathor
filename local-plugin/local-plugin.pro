#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:05:21
#
#-------------------------------------------------

QT       += core phonon xml network

TARGET = local-plugin-for-hathor
TEMPLATE = lib
CONFIG += plugin oauth

win32:LIBS += -llastfm0
debug:win32:DESTDIR = ../hathor/debug/plugins
release:win32:DESTDIR = ../hathor/release/plugins
!win32:DESTDIR = ../hathor/plugins
debug:win32:LIBS += "../hathor/debug/hathor.lib"
release:win32:LIBS += "../hathor/release/hathor.lib"

SOURCES += hlocalprovider.cpp \
    hlocalintro.cpp

HEADERS += hlocalprovider.h \
    hlocalintro.h

INCLUDEPATH+="../libhathor"

FORMS += \
    hlocalintro.ui
