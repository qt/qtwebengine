TARGET = qtpdfium

CONFIG += staticlib hide_symbols warn_off rtti_off exceptions_off c++11

DEFINES += NOMINMAX

load(qt_helper_lib)

unix:!mac: CONFIG -= debug_and_release

VPATH += ../3rdparty/pdfium
VPATH += ../3rdparty/pdfium/third_party

system(python ../3rdparty/gyp2pri.py --gyp-var libjpeg_gyp_path=third_party/third_party.gyp --gyp-var pdf_use_skia=0 ../3rdparty/pdfium/pdfium.gyp pdfium $$OUT_PWD/pdfium.pri)
system(python ../3rdparty/gyp2pri.py ../3rdparty/pdfium/third_party/third_party.gyp fx_freetype $$OUT_PWD/freetype.pri)

include($$OUT_PWD/pdfium.pri)

DEFINES += FT2_BUILD_LIBRARY
include($$OUT_PWD/freetype.pri)

win32: LIBS_PRIVATE += -ladvapi32 -lgdi32 -luser32
mac: LIBS_PRIVATE += -framework AppKit -framework CoreFoundation
