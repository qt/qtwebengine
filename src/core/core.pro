TEMPLATE = subdirs

# core_gyp_generator.pro is a dummy .pro file that is used by qmake
# to generate our main .gyp file
core_gyp_generator.file = core_gyp_generator.pro

# gyp_configure_host.pro and gyp_configure_target.pro are phony pro files that
# extract things like compiler and linker from qmake
gyp_configure_host.file = gyp_configure_host.pro
gyp_configure_target.file = gyp_configure_target.pro
gyp_configure_target.depends = gyp_configure_host

# gyp_run.pro calls gyp through gyp_qtwebengine on the qmake step, and ninja on the make step.
gyp_run.file = gyp_run.pro
gyp_run.depends = core_gyp_generator gyp_configure_host gyp_configure_target

# This will take the compile output of ninja, and link+deploy the final binary.
core_module.file = core_module.pro
core_module.depends = gyp_run

SUBDIRS += core_gyp_generator \
           gyp_configure_host \
           gyp_configure_target \
           gyp_run \
           core_module

REPACK_DIR = $$OUT_PWD/$$getConfigDir()/gen/repack
locales.files = "$$REPACK_DIR/qtwebengine_locales/*"
locales.CONFIG += no_check_exist
locales.path = $$[QT_INSTALL_TRANSLATIONS]/qtwebengine_locales
resources.files = $$REPACK_DIR/qtwebengine_resources.pak
resources.CONFIG += no_check_exist
resources.path = $$[QT_INSTALL_DATA]

PLUGIN_EXTENSION = .so
PLUGIN_PREFIX = lib
macx: PLUGIN_PREFIX =
win32 {
    PLUGIN_EXTENSION = .dll
    PLUGIN_PREFIX =

    icu.files = $$OUT_PWD/$$getConfigDir()/icudt46l.dat
    icu.CONFIG += no_check_exist
    icu.path = $$[QT_INSTALL_BINS]
    INSTALLS += icu
}

plugins.files = $$OUT_PWD/$$getConfigDir()/$${PLUGIN_PREFIX}ffmpegsumo$${PLUGIN_EXTENSION}
plugins.CONFIG += no_check_exist
plugins.path = $$[QT_INSTALL_PLUGINS]/qtwebengine

INSTALLS += locales resources plugins

