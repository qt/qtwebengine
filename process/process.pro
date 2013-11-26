TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

macx {
    LIBPATH = $$getOutDir()/$$getConfigDir()
    CONFIG -= app_bundle
} else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$getOutDir()/$$getConfigDir()

INCLUDEPATH += ../lib

SOURCES = main.cpp
