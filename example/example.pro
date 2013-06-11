TEMPLATE = app
TARGET = example

HEADERS = quickwindow.h widgetwindow.h
SOURCES = quickwindow.cpp widgetwindow.cpp main.cpp

OTHER_FILES += quickwindow.qml

INCLUDEPATH += ../lib

LIBPATH = $$getOutDir()/$$getConfigDir()/lib

LIBS += -L$$LIBPATH -lQt5WebEngine
QMAKE_RPATHDIR += $$LIBPATH

QT += widgets quick
MOC_DIR=$$PWD

