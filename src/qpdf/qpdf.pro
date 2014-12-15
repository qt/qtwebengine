TARGET = QtQPdf
QT = gui core network
TEMPLATE = lib
CONFIG += c++11
INCLUDEPATH += ../3rdparty/pdfium/fpdfsdk/include
load(qt_module)

LIBS_PRIVATE += -L$$QT_BUILD_TREE/lib -lqtpdfium$$qtPlatformTargetSuffix()

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

SOURCES += \
    jsbridge.cpp
