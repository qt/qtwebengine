MODULE = webenginecore
TARGET = QtWebEngineCore

# We depend on libc++ to build chromium so our macosx-version-min has to be 10.7
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/$$getConfigDir()/$${TARGET}_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}

# We distribute the module binary but headers are only available in-tree.
CONFIG += no_module_headers
load(qt_module)

# Using -Wl,-Bsymbolic-functions seems to confuse the dynamic linker
# and doesn't let Chromium get access to libc symbols through dlsym.
CONFIG -= bsymbolic_functions

contains(QT_CONFIG, egl): CONFIG += egl

linux {
    CONFIG(release, debug|release) | contains(QT_CONFIG, separate_debug_info): QMAKE_POST_LINK="cd $(DESTDIR) && $(STRIP) --strip-unneeded $(TARGET)"
}
