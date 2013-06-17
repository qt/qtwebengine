INCLUDEPATH += $$absolute_path(../lib, $$PWD)

LIBPATH = $$getOutDir()/$$getConfigDir()/lib

LIBS += -L$$LIBPATH -lQt5WebEngine
QMAKE_RPATHDIR += $$LIBPATH

# Quick hack for now as we mess with that for the gyp generation step.
MOC_DIR=$$PWD/.moc

