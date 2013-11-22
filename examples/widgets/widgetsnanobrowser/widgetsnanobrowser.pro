TEMPLATE = app
TARGET = widgetsnanobrowser

macx: CONFIG -= app_bundle
qnx: QMAKE_LFLAGS += $$QMAKE_LFLAGS_RPATHLINK$$getOutDir()/$$getConfigDir()/lib


HEADERS = widgetwindow.h
SOURCES = widgetwindow.cpp main.cpp

RESOURCES += ../../common/common_resources.qrc

QT += webenginewidgets
