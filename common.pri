macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib

# Allows examples/tests to link to libQt5WebEngineCore which
# isn't deployed properly yet.
QMAKE_RPATHDIR += $$LIBPATH
