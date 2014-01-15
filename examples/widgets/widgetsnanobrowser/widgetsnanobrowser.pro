TEMPLATE = app
TARGET = widgetsnanobrowser

macx: CONFIG -= app_bundle

HEADERS = widgetwindow.h
SOURCES = widgetwindow.cpp main.cpp

RESOURCES += ../../common/common_resources.qrc

QT += webenginewidgets

target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgetsnanobrowser
INSTALLS += target
