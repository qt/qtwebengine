TEMPLATE = app
TARGET = example

SOURCES = main.cpp

INCLUDEPATH += ../lib

LIBS += -L../lib -lblinq

QT += widgets

QMAKE_RPATHDIR += ../lib
