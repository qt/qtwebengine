macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib

macx: CONFIG -= app_bundle

# Allows examples/tests to link to libQt5WebEngineCore which
# isn't deployed properly yet.
QMAKE_RPATHDIR += $$LIBPATH
