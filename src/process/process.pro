TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

QT_PRIVATE += webenginecore

CONFIG -= app_bundle

load(qt_build_paths)
DESTDIR = $$MODULE_BASE_OUTDIR/libexec

INCLUDEPATH += ../core

SOURCES = main.cpp

target.path = $$[QT_INSTALL_LIBEXECS]
INSTALLS += target
