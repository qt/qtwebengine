TARGET = qquickwebviewgraphics
include(../tests.pri)
TARGET = $${TARGET}_software
OBJECTS_DIR = $${OBJECTS_DIR}_software

DEFINES += TST_QQUICKWEBVIEWGRAPHICS_SOFTWARE=1