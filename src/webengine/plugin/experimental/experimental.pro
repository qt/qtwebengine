CXX_MODULE = qml
TARGET = qtwebengineexperimentalplugin
TARGETPATH = QtWebEngine/experimental
IMPORT_VERSION = 1.0

QT += webengine qml quick
QT_PRIVATE += webengine-private qml-private quick-private gui-private core-private

INCLUDEPATH += $$QTWEBENGINE_ROOT/lib $$QTWEBENGINE_ROOT/lib/quick

SOURCES = plugin.cpp

load(qml_plugin)
