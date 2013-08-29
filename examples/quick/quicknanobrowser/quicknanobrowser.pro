TEMPLATE = app
TARGET = quicknanobrowser

include($$QTWEBENGINE_ROOT/common.pri)

HEADERS = quickwindow.h
SOURCES = quickwindow.cpp \
          main.cpp

OTHER_FILES += ContextMenuExtras.qml \
               quickwindow.qml


RESOURCES += resources.qrc
RESOURCES += ../../common/common_resources.qrc

QT += qml quick
qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}
