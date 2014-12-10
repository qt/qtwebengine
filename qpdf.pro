TEMPLATE = lib
CONFIG += c++11 warn_off
VPATH += src/3rdparty/pdfium
linux*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF
include(pdfium.pri)
