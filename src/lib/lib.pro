TARGET = qtpdfium

CONFIG += staticlib hide_symbols warn_off rtti_off exceptions_off c++11

DEFINES += NOMINMAX

load(qt_helper_lib)

unix:!mac: CONFIG -= debug_and_release

VPATH += ../3rdparty/pdfium

system(python ../3rdparty/gyp2pri.py ../3rdparty/pdfium/pdfium.gyp pdfium $$OUT_PWD/pdfium.pri)

include($$OUT_PWD/pdfium.pri)

win32: LIBS_PRIVATE += -ladvapi32 -lgdi32 -luser32
mac: LIBS_PRIVATE += -framework AppKit -framework CoreFoundation
