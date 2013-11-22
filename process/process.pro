TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

qnx: QMAKE_LFLAGS           += $$QMAKE_LFLAGS_RPATHLINK$${QNX_DIR}/$${QNX_CPUDIR}/usr/lib/qt5/lib
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$getOutDir()/$$getConfigDir()

INCLUDEPATH += ../lib

SOURCES = main.cpp
