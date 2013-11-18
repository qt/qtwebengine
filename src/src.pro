TEMPLATE = subdirs

# The first three subdirs contain dummy .pro files that are used by qmake
# to generate a corresponding .gyp file

# qmake_extras contains a phony pro file that extracts things like compiler and linker from qmake
# shared, core and process contain phony pro files that generate gyp files. Will be built by ninja.
shared.depends = qmake_extras
core.depends = qmake_extras
process.depends = qmake_extras

# This is where we use the generated gypi files and run gyp_qtwebengine
gypwrapper.file = gypwrapper.pro
gypwrapper.depends = resources shared core process

# API libraries
webengine.depends = gypwrapper
webenginewidgets.depends = gypwrapper
webengine_plugin.subdir = webengine/plugin
webengine_plugin.target = sub-webengine-plugin
webengine_plugin.depends = webengine
webengine_experimental_plugin.subdir = webengine/plugin/experimental
webengine_experimental_plugin.target = sub-webengine-experimental-plugin
webengine_experimental_plugin.depends = webengine


SUBDIRS += qmake_extras \
          resources \
          shared \
          core \
          process \
          gypwrapper \
          webengine \
          webengine_plugin \
          webengine_experimental_plugin

qtHaveModule(widgets) {
    SUBDIRS += webenginewidgets
}
