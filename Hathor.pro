HEADERS += \
    hauthaction.h \
    haction.h \
    hbrowser.h \
    hloginwidget.h \
    hbackground.h \
    lastfmext.h \
    hgraphicsview.h \
    hartist.h \
    halbumbox.h \
    halbum.h \
    htrackbox.h \
    htrack.h \
    hstupidresizewidget.h \
    hartistcontext.h \
    htagbox.h \
    htag.h \
    hartistbox.h \
    huser.h \
    huserbox.h \
    hshout.h \
    hshoutbox.h \
    htoolbar.h \
    halbumcontext.h \
    htrackcontext.h \
    hgrowingwidget.h \
    hrdiointerface.h \
    hplayercontext.h \
    kfadewidgeteffect_p.h \
    kfadewidgeteffect.h \
    hsearchcontext.h \
    hobject.h \
    hslideshow.h \
    hmainwindow.h \
    hmaincontext.h \
    hnotifier.h

SOURCES += \
    hauthaction.cpp \
    haction.cpp \
    hbrowser.cpp \
    hloginwidget.cpp \
    hbackground.cpp \
    main.cpp \
    lastfmext.cpp \
    hgraphicsview.cpp \
    hartist.cpp \
    halbumbox.cpp \
    halbum.cpp \
    htrackbox.cpp \
    htrack.cpp \
    hartistcontext.cpp \
    htagbox.cpp \
    htag.cpp \
    hartistbox.cpp \
    huser.cpp \
    huserbox.cpp \
    hshout.cpp \
    hshoutbox.cpp \
    htoolbar.cpp \
    halbumcontext.cpp \
    htrackcontext.cpp \
    hgrowingwidget.cpp \
    hrdiointerface.cpp \
    hplayercontext.cpp \
    kfadewidgeteffect.cpp \
    hsearchcontext.cpp \
    hobject.cpp \
    hslideshow.cpp \
    hmainwindow.cpp \
    hmaincontext.cpp

!win32:LIBS += -loauth -llastfm -lqjson
win32:LIBS += -llastfm0

CONFIG += oauth

QT += xml network webkit

FORMS += \
    hloginwidget.ui \
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
    hsearchcontext.ui

RESOURCES += \
    icons.qrc
