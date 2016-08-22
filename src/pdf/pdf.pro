TARGET = QtQPdf
QT += gui core core-private
QT_PRIVATE += network
TEMPLATE = lib
CONFIG += c++11
CONFIG -= precompile_header # Not supported by upstream header files
win32: DEFINES += NOMINMAX
INCLUDEPATH += ../3rdparty/pdfium/fpdfsdk/include
INCLUDEPATH += ../3rdparty/pdfium
INCLUDEPATH += ../3rdparty/pdfium/third_party/freetype/include
load(qt_module)

LIBS_PRIVATE += -L$$MODULE_BASE_OUTDIR/lib -lqtpdfium$$qtPlatformTargetSuffix()

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON += -wd"4100"
}

SOURCES += \
    jsbridge.cpp \
    qpdfbookmarkmodel.cpp \
    qpdfdocument.cpp

HEADERS += \
    qpdfbookmarkmodel.h \
    qpdfdocument.h \
    qpdfdocument_p.h \
    qtpdfglobal.h
