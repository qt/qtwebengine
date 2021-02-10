TEMPLATE = app

QT += quick webenginequick
CONFIG += c++11

SOURCES += \
    main.cpp

HEADERS += \
    utils.h

RESOURCES += qml.qrc

!cross_compile {
    DEFINES += DESKTOP_BUILD
    SOURCES += touchmockingapplication.cpp
    HEADERS += touchmockingapplication.h
    QT += gui-private
}
