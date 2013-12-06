TEMPLATE = app
TARGET = quicknanobrowser

macx: CONFIG -= app_bundle

HEADERS = quickwindow.h
SOURCES = quickwindow.cpp \
          main.cpp

OTHER_FILES += ContextMenuExtras.qml \
               quickwindow.qml

RESOURCES += resources.qrc
RESOURCES += ../../common/common_resources.qrc

QT += qml quick
QT_PRIVATE += quick-private gui-private core-private

qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}
