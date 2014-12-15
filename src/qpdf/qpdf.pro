TARGET = QtQPdf
QT = gui core network
TEMPLATE = lib
CONFIG += c++11 warn_off
VPATH += ../3rdparty/pdfium
INCLUDEPATH += ../3rdparty/pdfium/fpdfsdk/include
include(../3rdparty/pdfium.pri)
load(qt_module)

SOURCES += \
    jsbridge.cpp
