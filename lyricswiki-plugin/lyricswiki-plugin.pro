#-------------------------------------------------
#
# Project created by QtCreator 2012-02-23T18:36:57
#
#-------------------------------------------------

QT       += core phonon xml network webkit

TARGET = lyricswiki-plugin
TEMPLATE = lib

DEFINES += LYRICSWIKIPLUGIN_LIBRARY

SOURCES += lyricswikiplugin.cpp

HEADERS += lyricswikiplugin.h\
        lyricswiki-plugin_global.h

debug:win32:DESTDIR = ../hathor/debug/plugins
release:win32:DESTDIR = ../hathor/release/plugins
!win32:DESTDIR = ../hathor/plugins/
debug:win32:LIBS += "../hathor/debug/hathor.lib"
release:win32:LIBS += "../hathor/release/hathor.lib"

INCLUDEPATH+="../libhathor"
