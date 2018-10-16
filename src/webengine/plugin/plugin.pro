CXX_MODULE = qml
TARGET = qtwebengineplugin
TARGETPATH = QtWebEngine
IMPORT_VERSION = 1.8

QT += qml quick
QT_PRIVATE += core-private webenginecore-private webengine-private

SOURCES = plugin.cpp

load(qml_plugin)
