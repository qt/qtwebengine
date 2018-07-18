TARGET = QtPdf
MODULE = pdf

QT += gui core core-private
QT_PRIVATE += network

TEMPLATE = lib

CONFIG += c++11
CONFIG -= precompile_header # Not supported by upstream header files

INCLUDEPATH += $$QTWEBENGINE_ROOT/src/pdf
CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
CHROMIUM_GEN_DIR = $$OUT_PWD/../$$getConfigDir()/gen
INCLUDEPATH += $$QTWEBENGINE_ROOT/src/pdf \
               $$CHROMIUM_GEN_DIR \
               $$CHROMIUM_SRC_DIR \
               api

DEFINES += QT_BUILD_PDF_LIB
win32: DEFINES += NOMINMAX

linking_pri = $$OUT_PWD/$$getConfigDir()/$${TARGET}.pri
!include($$linking_pri) {
    error("Could not find the linking information that gn should have generated.")
}

isEmpty(NINJA_OBJECTS): error("Missing object files from QtPdf linking pri.")
isEmpty(NINJA_LFLAGS): error("Missing linker flags from QtPdf linking pri")
isEmpty(NINJA_ARCHIVES): error("Missing archive files from QtPdf linking pri")
isEmpty(NINJA_LIBS): error("Missing library files from QtPdf linking pri")

NINJA_OBJECTS = $$eval($$list($$NINJA_OBJECTS))
RSP_FILE = $$OUT_PWD/$$getConfigDir()/$${TARGET}.rsp
for(object, NINJA_OBJECTS): RSP_CONTENT += $$object
write_file($$RSP_FILE, RSP_CONTENT)

macos:LIBS_PRIVATE += -Wl,-filelist,$$shell_quote($$RSP_FILE)
linux:LIBS_PRIVATE += @$$RSP_FILE

# QTBUG-58710 add main rsp file on windows
win32:QMAKE_LFLAGS += @$$RSP_FILE

linux: LIBS_PRIVATE += -Wl,--start-group $$NINJA_ARCHIVES -Wl,--end-group
else: LIBS_PRIVATE += $$NINJA_ARCHIVES

LIBS_PRIVATE += $$NINJA_LIB_DIRS $$NINJA_LIBS

QMAKE_DOCS = $$PWD/doc/qtpdf.qdocconf

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON += -wd"4100"
}

SOURCES += \
    qpdfbookmarkmodel.cpp \
    qpdfdocument.cpp \
    qpdfpagenavigation.cpp \
    qpdfpagerenderer.cpp

# all "public" headers must be in "api" for sync script and to hide auto generated headers
# by Chromium in case of in-source build

HEADERS += \
    api/qpdfbookmarkmodel.h \
    api/qpdfdocument.h \
    api/qpdfdocument_p.h \
    api/qpdfdocumentrenderoptions.h \
    api/qpdfnamespace.h \
    api/qpdfpagenavigation.h \
    api/qpdfpagerenderer.h \
    api/qtpdfglobal.h \
    api/qpdfdocument_p.h

load(qt_module)
