TEMPLATE = subdirs

process.depends = core
webengine.depends = core
webenginewidgets.depends = core
webengine_plugin.subdir = webengine/plugin
webengine_plugin.target = sub-webengine-plugin
webengine_plugin.depends = webengine
webengine_experimental_plugin.subdir = webengine/plugin/experimental
webengine_experimental_plugin.target = sub-webengine-experimental-plugin
webengine_experimental_plugin.depends = webengine


SUBDIRS += core \
           process \
           webengine \
           webengine_plugin \
           webengine_experimental_plugin

qtHaveModule(widgets) {
    SUBDIRS += webenginewidgets
}
