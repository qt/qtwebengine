TEMPLATE = app
TARGET = widgetsnanobrowser

macx: CONFIG -= app_bundle

HEADERS = widgetwindow.h
SOURCES = widgetwindow.cpp main.cpp

RESOURCES += ../../common/common_resources.qrc

QT += webenginewidgets

CONFIG -= qt_example_installs
