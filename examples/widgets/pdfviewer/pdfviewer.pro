QT       += core gui widgets pdf
TARGET = pdfviewer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    sequentialpagewidget.cpp

HEADERS  += mainwindow.h \
    sequentialpagewidget.h

FORMS    += mainwindow.ui
