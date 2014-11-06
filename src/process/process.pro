TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

QT_PRIVATE += webenginecore

load(qt_build_paths)
contains(QT_CONFIG, qt_framework) {
    # Deploy the QtWebEngineProcess app bundle into the QtWebEngineCore framework.
    DESTDIR = $$MODULE_BASE_OUTDIR/lib/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    CONFIG -= app_bundle
    DESTDIR = $$MODULE_BASE_OUTDIR/libexec
}

INCLUDEPATH += ../core

SOURCES = main.cpp

contains(QT_CONFIG, qt_framework) {
    target.path = $$[QT_INSTALL_LIBS]/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    target.path = $$[QT_INSTALL_LIBEXECS]
}
INSTALLS += target
