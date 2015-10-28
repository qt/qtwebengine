TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app
!build_pass:contains(QT_CONFIG, debug_and_release):contains(QT_CONFIG, build_all): CONFIG += release
# Needed to set LSUIElement=1
QMAKE_INFO_PLIST = Info_mac.plist

load(qt_build_paths)
contains(QT_CONFIG, qt_framework) {
    # Deploy the QtWebEngineProcess app bundle into the QtWebEngineCore framework.
    DESTDIR = $$MODULE_BASE_OUTDIR/lib/QtWebEngineCore.framework/Versions/5/Helpers

    QT += webenginecore
} else {
    CONFIG -= app_bundle
    win32: DESTDIR = $$MODULE_BASE_OUTDIR/bin
    else:  DESTDIR = $$MODULE_BASE_OUTDIR/libexec

    QT_PRIVATE += webenginecore
}

msvc: QMAKE_LFLAGS *= /LARGEADDRESSAWARE

INCLUDEPATH += ../core

SOURCES = main.cpp

win32 {
    SOURCES += \
        support_win.cpp
}

contains(QT_CONFIG, qt_framework) {
    target.path = $$[QT_INSTALL_LIBS]/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    target.path = $$[QT_INSTALL_LIBEXECS]
}
INSTALLS += target
