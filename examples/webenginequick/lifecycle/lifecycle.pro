TEMPLATE = app

QT += qml quick quickcontrols2 webenginequick
CONFIG += qmltypes
QML_IMPORT_NAME = LifecycleUtils
QML_IMPORT_MAJOR_VERSION = 1

HEADERS += utils.h
SOURCES += main.cpp

RESOURCES += resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginequick/lifecycle
INSTALLS += target
