include(../tests.pri)
include(../../shared/https.pri)
QT *= webenginecore-private webenginequick webenginequick-private
HEADERS += $$PWD/testhandler.h
SOURCES += $$PWD/testhandler.cpp
RESOURCES += certificateerror.qrc

