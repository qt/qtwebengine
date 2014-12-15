TARGET = QtQPdf
QT = gui core network
TEMPLATE = lib
CONFIG += c++11
INCLUDEPATH += ../3rdparty/pdfium/fpdfsdk/include
load(qt_module)

LIBS_PRIVATE += -L$$MODULE_BASE_OUTDIR/lib -lqtpdfium$$qtPlatformTargetSuffix()

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON += -wd"4100"
}

SOURCES += \
    jsbridge.cpp
