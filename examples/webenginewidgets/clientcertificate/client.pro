TEMPLATE = app

QT += webenginewidgets

SOURCES += client.cpp

RESOURCES += resources/client.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/clientcertificate/client
INSTALLS += target
