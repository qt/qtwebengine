requires(qtConfig(accessibility))

TEMPLATE = app
TARGET = quicknanobrowser

HEADERS = utils.h
SOURCES = main.cpp

RESOURCES += resources.qrc

QT += qml quick webenginequick

CONFIG += qmltypes
QML_IMPORT_NAME = BrowserUtils
QML_IMPORT_MAJOR_VERSION = 1

qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}

target.path = $$[QT_INSTALL_EXAMPLES]/webenginequick/quicknanobrowser
INSTALLS += target
