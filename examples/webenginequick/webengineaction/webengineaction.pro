TEMPLATE = app

QT += webenginequick

HEADERS += utils.h
SOURCES += main.cpp

RESOURCES += qml.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginequick/webengineaction
INSTALLS += target
