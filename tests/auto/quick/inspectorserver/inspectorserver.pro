include(../tests.pri)
CONFIG -= testcase      # remove, once this passes in the CI
QT += webengine
QT_PRIVATE += webengine-private
DEFINES += IMPORT_DIR=\"\\\"$${ROOT_BUILD_DIR}$${QMAKE_DIR_SEP}imports\\\"\"
