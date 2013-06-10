TEMPLATE = app
TARGET = example

HEADERS = quickwindow.h widgetwindow.h
SOURCES = quickwindow.cpp widgetwindow.cpp main.cpp

INCLUDEPATH += ../lib

LIBPATH = $$getOutDir()/$$getConfigDir()/lib

LIBS += -L$$LIBPATH -lblinq
QMAKE_RPATHDIR += $$LIBPATH

QT += widgets quick
MOC_DIR=$$PWD
