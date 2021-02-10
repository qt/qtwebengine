include(../tests.pri)
QT += webenginequick
QT_PRIVATE += core-private webenginequick-private webenginecore-private
DEFINES += IMPORT_DIR=\"\\\"$${ROOT_BUILD_DIR}$${QMAKE_DIR_SEP}imports\\\"\"
