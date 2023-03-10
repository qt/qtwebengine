TEMPLATE = app

QT += core network
CONFIG += console

SOURCES += server.cpp

RESOURCES += resources/server.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/clientcertificate/server
INSTALLS += target
