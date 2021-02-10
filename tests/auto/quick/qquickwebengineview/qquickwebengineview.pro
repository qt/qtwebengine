include(../tests.pri)

exists($${TARGET}.qrc):RESOURCES += $${TARGET}.qrc
QT_PRIVATE += core_private gui-private webenginequick-private webenginecore-private

HEADERS += ../shared/util.h
