TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$getOutDir()/$$getConfigDir()

INCLUDEPATH += ../lib

SOURCES = main.cpp

target.files = $$DESTDIR/$$QTWEBENGINEPROCESS_NAME
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
