TEMPLATE = app
TARGET = example

SOURCES = main.cpp

INCLUDEPATH += ../lib

LIBPATH = $$getOutDir()

LIBS += -L$$LIBPATH/lib -lblinq
QMAKE_RPATHDIR += $$LIBPATH

QT += widgets
