TEMPLATE = app

DEFINES += WIDGET_TOUCHBROWSER
QT += core gui webenginewidgets

INCLUDEPATH += ../../touchmocking

SOURCES += \
    main.cpp \
    ../../touchmocking/touchmockingapplication.cpp
HEADERS += \
    ../../touchmocking/touchmockingapplication.h \
    ../../touchmocking/utils.h

RESOURCES += resources.qrc
