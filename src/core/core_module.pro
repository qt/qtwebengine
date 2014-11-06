MODULE = webenginecore
TARGET = QtWebEngineCore

CMAKE_MODULE_TESTS = "-"

QT += qml quick
QT_PRIVATE += gui-private

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/$$getConfigDir()/$${TARGET}_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}

REPACK_DIR = $$OUT_PWD/$$getConfigDir()/gen/repack
locales.files = "$$REPACK_DIR/qtwebengine_locales/*"
locales.CONFIG += no_check_exist
locales.path = $$[QT_INSTALL_TRANSLATIONS]/qtwebengine_locales
resources.files = $$REPACK_DIR/qtwebengine_resources.pak
resources.CONFIG += no_check_exist
resources.path = $$[QT_INSTALL_DATA]

PLUGIN_EXTENSION = .so
PLUGIN_PREFIX = lib
osx: PLUGIN_PREFIX =
win32 {
    PLUGIN_EXTENSION = .dll
    PLUGIN_PREFIX =
}

icu.files = $$OUT_PWD/$$getConfigDir()/icudtl.dat
icu.CONFIG += no_check_exist
icu.path = $$[QT_INSTALL_DATA]

plugins.files = $$OUT_PWD/$$getConfigDir()/$${PLUGIN_PREFIX}ffmpegsumo$${PLUGIN_EXTENSION}
plugins.CONFIG += no_check_exist
plugins.path = $$[QT_INSTALL_PLUGINS]/qtwebengine

INSTALLS += icu locales resources plugins

# We distribute the module binary but headers are only available in-tree.
CONFIG += no_module_headers
load(qt_module)

# Using -Wl,-Bsymbolic-functions seems to confuse the dynamic linker
# and doesn't let Chromium get access to libc symbols through dlsym.
CONFIG -= bsymbolic_functions

contains(QT_CONFIG, egl): CONFIG += egl

linux: contains(QT_CONFIG, separate_debug_info): QMAKE_POST_LINK="cd $(DESTDIR) && $(STRIP) --strip-unneeded $(TARGET)"
