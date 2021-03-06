include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore webenginecore-private

TEMPLATE = subdirs

SUBDIRS += \
    defaultsurfaceformat \
    faviconmanager \
    loadsignals \
    offscreen \
    proxy \
    proxypac \
    schemes \
    shutdown \
    qwebenginedownloadrequest \
    qwebenginepage \
    qwebenginehistory \
    qwebengineprofile \
    qwebenginescript \
    qwebengineview

# Synthetic touch events are not supported on macOS
!macos: SUBDIRS += touchinput

qtConfig(accessibility) {
    SUBDIRS += accessibility
}

qtConfig(webengine-printing-and-pdf) {
    SUBDIRS += printing
}

qtConfig(webengine-spellchecker):!cross_compile {
    !qtConfig(webengine-native-spellchecker) {
        SUBDIRS += spellchecking
    } else {
        message("Spellcheck test will not be built because it depends on usage of Hunspell dictionaries.")
    }
}

# QTBUG-60268
boot2qt: SUBDIRS -= accessibility defaultsurfaceformat devtools \
                    qwebenginepage \
                    qwebengineprofile  \
                    qwebengineview

darwin|win32: SUBDIRS -= offscreen
