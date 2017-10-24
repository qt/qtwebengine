include(../tests.pri)

exists($${TARGET}.qrc):RESOURCES += $${TARGET}.qrc
QT_PRIVATE += webengine-private gui-private

HEADERS += ../shared/util.h

qtConfig(webengine-printing-and-pdf) {
    DEFINES += ENABLE_PDF
}

