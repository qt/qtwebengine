TEMPLATE = app
TARGET = widget-nano-browser

include(../common.pri)

HEADERS = widgetwindow.h
SOURCES = widgetwindow.cpp main.cpp

# FIXME: in the long run, we should only have 'QT += webenginewidgets' in here
#LIBS += -lQt5WebEngineWidgets
#INCLUDEPATH += $$QTWEBENGINE_ROOT/lib/widgets
#QT += widgets
QT += webenginewidgets
