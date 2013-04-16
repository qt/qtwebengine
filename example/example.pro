TEMPLATE = app
TARGET = example

SOURCES = main.cpp

INCLUDEPATH += ../lib

LIBS += -L../out/Release/lib -lblinq

#QT += widgets

QMAKE_RPATHDIR += ../out/Release/lib
