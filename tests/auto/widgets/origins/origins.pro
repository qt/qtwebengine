include(../tests.pri)
CONFIG += c++14
qtHaveModule(websockets) {
    QT += websockets
    DEFINES += WEBSOCKETS
}

