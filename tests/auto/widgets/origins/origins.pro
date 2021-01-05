include(../tests.pri)
include(../../shared/http.pri)
CONFIG += c++14
qtConfig(webengine-webchannel):qtHaveModule(websockets) {
    QT += websockets
    DEFINES += WEBSOCKETS
}

