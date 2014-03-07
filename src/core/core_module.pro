TARGET = QtWebEngineCore
MODULE = webenginecore

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/$$getConfigDir()/$${TARGET}_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}

QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private

# We distribute the module binary but headers are only available in-tree.
CONFIG += no_module_headers
load(qt_module)

# Using -Wl,-Bsymbolic-functions seems to confuse the dynamic linker
# and doesn't let Chromium get access to libc symbols through dlsym.
CONFIG -= bsymbolic_functions
