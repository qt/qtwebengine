QT       += core gui widgets pdf
TARGET = pdfviewer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    sequentialpagewidget.cpp \
    pagerenderer.cpp

HEADERS  += mainwindow.h \
    sequentialpagewidget.h \
    pagerenderer.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
