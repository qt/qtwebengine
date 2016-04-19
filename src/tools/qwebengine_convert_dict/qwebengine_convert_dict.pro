option(host_build)

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/../../core/$$getConfigDir()/QtWebEngineCore_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}
# remove object files from linking information
OBJECTS =

# Fixme: -Werror=unused-parameter in core
QMAKE_CXXFLAGS_WARN_ON =

# Issue with some template compliation, smb smart should look at it
win32: DEFINES += NOMINMAX

CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
INCLUDEPATH += $$CHROMIUM_SRC_DIR

SOURCES += \
    main.cpp

load(qt_tool)
