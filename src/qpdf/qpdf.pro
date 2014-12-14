TARGET = QtQPdf
QT = gui core network
TEMPLATE = lib
CONFIG += c++11 warn_off
VPATH += ../3rdparty/pdfium
include(pdfium.pri)
load(qt_module)
