include(../tests.pri)
QT += core-private webenginequick webenginequick-private

HEADERS += \
    server.h \
    testhandler.h

SOURCES += \
    server.cpp \
    testhandler.cpp

RESOURCES += \
    dialogs.qrc
