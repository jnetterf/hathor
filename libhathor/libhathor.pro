#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:31:50
#
#-------------------------------------------------

QT       += network webkit xml phonon

TARGET = hathor
TEMPLATE = lib

DEFINES += LIBHATHOR_LIBRARY

SOURCES += habstractmusicinterface.cpp \
    haction.cpp \
    halbum.cpp \
    hartist.cpp \
    hbrowser.cpp \
    hobject.cpp \
    hshout.cpp \
    htag.cpp \
    htrack.cpp \
    huser.cpp \
    lastfmext.cpp \
    hfuture.cpp

HEADERS += habstractmusicinterface.h\
        libhathor_global.h \
    haction.h \
    halbum.h \
    hartist.h \
    hbrowser.h \
    hobject.h \
    hnotifier.h \
    hshout.h \
    htag.h \
    htrack.h \
    huser.h \
    lastfmext.h \
    hfuture.h

CONFIG += oauth

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE5AF8F68
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = libhathor.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
