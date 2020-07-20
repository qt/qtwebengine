TARGET = QtPdfWidgets
QT = core gui widgets widgets-private pdf

SOURCES += \
    qpdfview.cpp

HEADERS += \
    qpdfview.h \
    qpdfview_p.h \
    qtpdfwidgetsglobal.h

include($${OUT_PWD}/../pdf/$$getConfigDir()/QtPdf_static_dep.pri)
load(qt_module)
