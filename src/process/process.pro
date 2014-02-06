TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

macx|win32 {
    LIBPATH = $$getOutDir()/$$getConfigDir()
    CONFIG -= app_bundle
} else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib

win:LIBS_PRIVATE += -L$$LIBPATH -lQt5WebEngineCore.dll
else:LIBS_PRIVATE += -L$$LIBPATH -lQt5WebEngineCore

QMAKE_RPATHDIR += $$LIBPATH

qnx: QMAKE_RPATHLINKDIR += $${QNX_DIR}/$${QNX_CPUDIR}/usr/lib/qt5/lib
else: cross_compile: QMAKE_RPATHLINKDIR += $$[QT_INSTALL_LIBS]

load(qt_build_paths)
DESTDIR = $$MODULE_BASE_OUTDIR/libexec

INCLUDEPATH += ../core

SOURCES = main.cpp

target.path = $$[QT_INSTALL_LIBEXECS]
INSTALLS += target
