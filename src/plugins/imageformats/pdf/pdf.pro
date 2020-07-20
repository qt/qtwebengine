TARGET  = qpdf
PLUGIN_TYPE = imageformats
PLUGIN_EXTENDS = pdf
PLUGIN_CLASS_NAME = QPdfPlugin

HEADERS += qpdfiohandler_p.h
SOURCES += main.cpp \
           qpdfiohandler.cpp
QT += pdf

include($${OUT_PWD}/../../../pdf/$$getConfigDir()/QtPdf_static_dep.pri)

load(qt_plugin)
