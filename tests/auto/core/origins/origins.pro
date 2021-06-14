include(../tests.pri)
include(../../shared/http.pri)

qtConfig(webengine-webchannel):qtHaveModule(websockets) {
    QT += websockets
    DEFINES += WEBSOCKETS
}

