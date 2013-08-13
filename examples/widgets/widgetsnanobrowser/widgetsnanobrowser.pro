TEMPLATE = app
TARGET = widgetsnanobrowser

include($$QTWEBENGINE_ROOT/common.pri)

HEADERS = widgetwindow.h
SOURCES = widgetwindow.cpp main.cpp

RESOURCES += $$absolute_path(../../common/common_resources.qrc)

QT += webenginewidgets
