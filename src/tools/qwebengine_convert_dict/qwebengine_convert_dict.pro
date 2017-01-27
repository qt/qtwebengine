option(host_build)

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/../../core/$$getConfigDir()/convert_dict_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}

# skip dummy main.cpp file
OBJECTS =

# Fixme: -Werror=unused-parameter in core
QMAKE_CXXFLAGS_WARN_ON =

# Disable MSVC2015 warning C4577 ('noexcept' used with no exception handling mode specified)
win32: QMAKE_CXXFLAGS_WARN_ON = -wd4577

# Issue with some template compliation, smb smart should look at it
win32: DEFINES += NOMINMAX

CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
INCLUDEPATH += $$CHROMIUM_SRC_DIR

SOURCES += \
    main.cpp

# Support converting dictionaries in a prefix build, by supplying
# the path to the ICU data file located in the Qt build path, rather
# than the install path (which is not present at build time).
icu_data_dir.name = QT_WEBENGINE_ICU_DATA_DIR
icu_data_dir.value = $$OUT_PWD/../../../src/core/$$getConfigDir()
QT_TOOL_ENV = icu_data_dir
load(qt_tool)
QT_TOOL_ENV =
