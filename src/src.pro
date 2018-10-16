include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore webenginecore-private

TEMPLATE = subdirs

process.depends = core
webengine.depends = core
webenginewidgets.depends = core webengine
webengine_plugin.subdir = webengine/plugin
webengine_plugin.target = sub-webengine-plugin
webengine_plugin.depends = webengine

core.depends = buildtools

SUBDIRS += buildtools \
           core \
           process

qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
    SUBDIRS += qwebengine_convert_dict
    qwebengine_convert_dict.subdir = tools/qwebengine_convert_dict
    qwebengine_convert_dict.depends = core
}

qtConfig(webengine-qml) {
   SUBDIRS += webengine
}

qtConfig(webengine-widgets) {
   SUBDIRS += plugins webenginewidgets
   plugins.depends = webenginewidgets
}
