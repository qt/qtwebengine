TEMPLATE = subdirs

qml_module.file = module.pro
qml_plugin.file = plugin/plugin.pro

qml_plugin.depends = qml_module

SUBDIRS += qml_module qml_plugin

qtConfig(webenginequick-ui-delegates) {
    SUBDIRS += ui
}
