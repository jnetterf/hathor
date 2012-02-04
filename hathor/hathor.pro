HEADERS += \
    hloginwidget.h \
    hbackground.h \
    hgraphicsview.h \
    halbumbox.h \
    htrackbox.h \
    hstupidresizewidget.h \
    hartistcontext.h \
    htagbox.h \
    hartistbox.h \
    huserbox.h \
    hshoutbox.h \
    htoolbar.h \
    halbumcontext.h \
    htrackcontext.h \
    hgrowingwidget.h \
    hplayercontext.h \
    kfadewidgeteffect_p.h \
    kfadewidgeteffect.h \
    hsearchcontext.h \
    hslideshow.h \
    hmainwindow.h \
    hmaincontext.h \
    hplaywidget.h

SOURCES += \
    hloginwidget.cpp \
    hbackground.cpp \
    main.cpp \
    hgraphicsview.cpp \
    halbumbox.cpp \
    htrackbox.cpp \
    hartistcontext.cpp \
    htagbox.cpp \
    hartistbox.cpp \
    huserbox.cpp \
    hshoutbox.cpp \
    htoolbar.cpp \
    halbumcontext.cpp \
    htrackcontext.cpp \
    hgrowingwidget.cpp \
    hplayercontext.cpp \
    kfadewidgeteffect.cpp \
    hsearchcontext.cpp \
    hslideshow.cpp \
    hmainwindow.cpp \
    hmaincontext.cpp \
    hplaywidget.cpp

!win32:LIBS += -llastfm -L./libhathor -L../libhathor -lhathor
debug:win32:LIBS += -llastfm0 ./debug/hathor.lib
release:win32:LIBS += -llastfm0 ./release/hathor.lib

!win32:QMAKE_LFLAGS += -Wl,-rpath=\'\$\$ORIGIN\'/../libhathor

CONFIG += oauth

QT += xml network webkit phonon

FORMS += \
    artistcontext.ui \
    halbumbox.ui \
    htrackbox.ui \
    htagbox.ui \
    hartistbox.ui \
    huserbox.ui \
    hshoutbox.ui \
    htoolbar.ui \
    halbumcontext.ui \
    htrackcontext.ui \
    hplayercontext.ui \
    hsearchcontext.ui \
    hmainwindow.ui \
    hplaywidget.ui

RESOURCES += \
    icons.qrc

OTHER_FILES += \
    styleSheet.txt \
    win_rc.rc \
    hathor-logo.ico

RC_FILE = ./win_rc.rc

INCLUDEPATH += "./libhathor" "../libhathor"

