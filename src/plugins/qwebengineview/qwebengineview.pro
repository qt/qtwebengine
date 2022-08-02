TARGET      = qwebengineview
QT         += designer webenginewidgets

PLUGIN_CLASS_NAME = QWebEngineViewPlugin
PLUGIN_TYPE = designer
load(qt_plugin)

SOURCES += qwebengineview_plugin.cpp
HEADERS += qwebengineview_plugin.h
RESOURCES += qwebengineview_plugin.qrc
