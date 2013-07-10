INCLUDEPATH += $$absolute_path(../lib, $$PWD) \
               $$absolute_path(common, $$PWD)

macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib

HEADERS += common/util.h

RESOURCES += $$absolute_path(common/common_resources.qrc)

QMAKE_RPATHDIR += $$LIBPATH

# Quick hack for now as we mess with that for the gyp generation step.
MOC_DIR=$$PWD/.moc

