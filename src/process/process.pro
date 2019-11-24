TARGET = $$QTWEBENGINEPROCESS_NAME

# Needed to set LSUIElement=1
QMAKE_INFO_PLIST = Info_mac.plist

QT_PRIVATE += core-private webenginecore-private

INCLUDEPATH += ../core

SOURCES = main.cpp

CONFIG -= ltcg

# On windows we need to statically link to the windows sandbox code
win32 {
    # The Chromium headers we include are not clean
    CONFIG -= warnings_are_errors

    # Look for linking information produced by GN
    linking_pri = $$OUT_PWD/../core/$$getConfigDir()/qtwebengine_sandbox_win.pri

    !include($$linking_pri) {
        error("Could not find the linking information that gn should have generated.")
    }
    isEmpty(NINJA_OBJECTS): error("//sandbox/win:sandbox linking changed, update process.pro")
    isEmpty(NINJA_ARCHIVES): error("//sandbox/win:sandbox linking changed, update process.pro")

    LIBS_PRIVATE += $$NINJA_LIB_DIRS $$NINJA_LIBS $$NINJA_ARCHIVES $$NINJA_OBJECTS
    QMAKE_LFLAGS += $$NINJA_LFLAGS
    POST_TARGETDEPS += $$eval($$NINJA_TARGETDEPS)

    CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
    INCLUDEPATH += $$CHROMIUM_SRC_DIR \
                   $$OUT_PWD/../core/$$getConfigDir()/gen

    SOURCES += \
        support_win.cpp

    msvc: QMAKE_LFLAGS += /MANIFESTINPUT:$$PWD/process.exe.manifest
}

load(qt_app)

qtConfig(build_all): CONFIG += build_all

qtConfig(framework) {
    # Deploy the QtWebEngineProcess app bundle into the QtWebEngineCore framework.
    DESTDIR = $$MODULE_BASE_OUTDIR/lib/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    CONFIG -= app_bundle
    win32: DESTDIR = $$MODULE_BASE_OUTDIR/bin
    else:  DESTDIR = $$MODULE_BASE_OUTDIR/libexec
}
msvc: QMAKE_LFLAGS *= /LARGEADDRESSAWARE

qtConfig(framework) {
    target.path = $$[QT_INSTALL_LIBS]/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    target.path = $$[QT_INSTALL_LIBEXECS]
}
