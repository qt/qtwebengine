TARGET  = qpdf

PLUGIN_TYPE = imageformats
PLUGIN_EXTENDS = pdf
PLUGIN_CLASS_NAME = QPdfPlugin
load(qt_plugin)

HEADERS += qpdfiohandler_p.h
SOURCES += main.cpp \
           qpdfiohandler.cpp
QT += pdf
