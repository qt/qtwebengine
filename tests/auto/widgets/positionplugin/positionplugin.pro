TARGET = qtwebengine_positioning_testplugin

QT += positioning

SOURCES += plugin.cpp

OTHER_FILES += \
    plugin.json

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = TestPositionPlugin
PLUGIN_EXTENDS = -
load(qt_plugin)
