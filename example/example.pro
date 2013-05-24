TEMPLATE = app
TARGET = example

SOURCES = main.cpp

INCLUDEPATH += ../lib

LIBPATH = $$getOutDir()/$$getConfigDir()/lib

LIBS += -L$$LIBPATH -lblinq
QMAKE_RPATHDIR += $$LIBPATH

QT += widgets
