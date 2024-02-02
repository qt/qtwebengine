TEMPLATE = app

DEFINES += QUICK_TOUCHBROWSER
QT += core gui quick webenginequick

INCLUDEPATH += ../../touchmocking

SOURCES += \
    main.cpp \
    ../../touchmocking/touchmockingapplication.cpp
HEADERS += \
    ../../touchmocking/touchmockingapplication.h \
    ../../touchmocking/utils.h

RESOURCES += resources.qrc
