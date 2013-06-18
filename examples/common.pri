INCLUDEPATH += $$absolute_path(../lib, $$PWD) \
               $$absolute_path(common, $$PWD)

LIBPATH = $$getOutDir()/$$getConfigDir()/lib

HEADERS += common/util.h

LIBS += -L$$LIBPATH -lQt5WebEngine
QMAKE_RPATHDIR += $$LIBPATH

# Quick hack for now as we mess with that for the gyp generation step.
MOC_DIR=$$PWD/.moc

