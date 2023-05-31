TEMPLATE = app

QT += quickcontrols2 webenginequick

HEADERS += utils.h
SOURCES += main.cpp

RESOURCES += resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginequick/lifecycle
INSTALLS += target
