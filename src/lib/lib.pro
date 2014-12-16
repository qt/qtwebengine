TARGET = qtpdfium

CONFIG += static hide_symbols warn_off rtti_off exceptions_off c++11

DEFINES += NOMINMAX

load(qt_helper_lib)

VPATH += ../3rdparty/pdfium
include(../3rdparty/pdfium.pri)

win32: LIBS_PRIVATE += -ladvapi32 -lgdi32 -luser32
mac: LIBS_PRIVATE += -framework AppKit -framework CoreFoundation
