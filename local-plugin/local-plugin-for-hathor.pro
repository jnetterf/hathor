#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:05:21
#
#-------------------------------------------------

QT       += core phonon xml network

TARGET = local-plugin-for-hathor
TEMPLATE = lib
CONFIG += plugin oauth

DESTDIR = $$[QT_INSTALL_PLUGINS]/codecs

SOURCES += hlocalprovider.cpp

HEADERS += hlocalprovider.h
symbian {
# Load predefined include paths (e.g. QT_PLUGINS_BASE_DIR) to be used in the pro-files
    load(data_caging_paths)
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEC0C5D19
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    pluginDeploy.sources = alternate-plugin-for-hathor.dll
    pluginDeploy.path = $$QT_PLUGINS_BASE_DIR/alternate-plugin-for-hathor
    DEPLOYMENT += pluginDeploy
}

INCLUDEPATH+="../hathor/libhathor"

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
