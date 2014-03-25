TEMPLATE = app
TARGET = quicknanobrowser

HEADERS = quickwindow.h \
          util.h
SOURCES = quickwindow.cpp \
          main.cpp

OTHER_FILES += quickwindow.qml

RESOURCES += resources.qrc

QT += qml quick
QT_PRIVATE += quick-private gui-private core-private

qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}

target.path = $$[QT_INSTALL_EXAMPLES]/webengine/quicknanobrowser
INSTALLS += target
