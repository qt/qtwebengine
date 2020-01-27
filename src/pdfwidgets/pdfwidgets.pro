TARGET = QtPdfWidgets
QT = core gui widgets widgets-private pdf

SOURCES += \
    qpdfview.cpp

HEADERS += \
    qpdfview.h \
    qpdfview_p.h \
    qtpdfwidgetsglobal.h

load(qt_module)
