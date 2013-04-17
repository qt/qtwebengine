TEMPLATE = app
TARGET = example

SOURCES = main.cpp

INCLUDEPATH += ../lib

CONFIG(debug, debug|release): LIBPATH = ../out/Debug/lib
else: LIBPATH = ../out/Release/lib

LIBS += -L$$LIBPATH -lblinq
QMAKE_RPATHDIR += $$LIBPATH
