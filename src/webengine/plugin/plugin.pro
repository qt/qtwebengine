CXX_MODULE = qml
TARGET = qtwebengineplugin
TARGETPATH = QtWebEngine
IMPORT_VERSION = 1.10

QT += qml quick
QT_PRIVATE += core-private webenginecore-private webengine-private

SOURCES = plugin.cpp

QMAKE_QMLPLUGINDUMP_FLAGS = -defaultplatform
load(qml_plugin)
