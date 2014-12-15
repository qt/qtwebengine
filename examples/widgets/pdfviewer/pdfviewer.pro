QT       += core gui widgets pdf
TARGET = pdfviewer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    sequentialpagewidget.cpp \
    pagecache.cpp

HEADERS  += mainwindow.h \
    sequentialpagewidget.h \
    pagecache.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
