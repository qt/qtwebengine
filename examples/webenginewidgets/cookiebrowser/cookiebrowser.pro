QT += core gui webenginewidgets
TARGET = cookiebrowser
TEMPLATE = app
CONFIG += c++11

SOURCES += \
    main.cpp\
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui \
    cookiedialog.ui \
    cookiewidget.ui

RESOURCES += \
    cookiebrowser.qrc
