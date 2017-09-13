include(../tests.pri)

exists($${TARGET}.qrc):RESOURCES += $${TARGET}.qrc
QT_PRIVATE += webengine-private gui-private

HEADERS += ../shared/util.h

qtConfig(printing-and-pdf) {
    DEFINES += ENABLE_PDF
}

