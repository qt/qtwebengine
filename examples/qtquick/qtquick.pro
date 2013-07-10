TEMPLATE = app
TARGET = qtquick-nano-browser

include(../common.pri)

HEADERS = quickwindow.h
SOURCES = quickwindow.cpp main.cpp

OTHER_FILES += quickwindow.qml
RESOURCES += resources.qrc


QT += qml quick
qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}
