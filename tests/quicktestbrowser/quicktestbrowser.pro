TEMPLATE = app
TARGET = quicktestbrowser

macx: CONFIG -= app_bundle

HEADERS = quickwindow.h \
          util.h
SOURCES = quickwindow.cpp \
          main.cpp

OTHER_FILES += ContextMenuExtras.qml \
               quickwindow.qml

RESOURCES += resources.qrc

QT += qml quick
QT_PRIVATE += quick-private gui-private core-private

qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}
