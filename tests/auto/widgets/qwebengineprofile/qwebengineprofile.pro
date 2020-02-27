include(../tests.pri)
include(../../shared/http.pri)
exists($${TARGET}.qrc):RESOURCES += $${TARGET}.qrc
QT *= core-private gui-private
