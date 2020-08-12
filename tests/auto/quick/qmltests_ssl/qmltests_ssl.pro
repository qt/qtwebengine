include(../tests.pri)
include(../../shared/https.pri)
QT += qmltest

IMPORTPATH += $$PWD/data

OTHER_FILES += $$PWD/data/tst_certificateError.qml

load(qt_build_paths)
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$re_escape($$PWD$${QMAKE_DIR_SEP}data)\\\"
