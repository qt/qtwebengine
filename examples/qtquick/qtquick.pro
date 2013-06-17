TEMPLATE = app
TARGET = qtquick-nano-browser

include(../common.pri)

HEADERS = quickwindow.h
SOURCES = quickwindow.cpp main.cpp

OTHER_FILES += quickwindow.qml

QT += quick \
      widgets # QApplication is required to get native styling with QtQuickControls
