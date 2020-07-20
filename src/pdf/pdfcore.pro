TARGET = QtPdf
MODULE = pdf

QT += gui core core-private
QT_PRIVATE += network

TEMPLATE = lib

INCLUDEPATH += $$QTWEBENGINE_ROOT/src/pdf
CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
CHROMIUM_GEN_DIR = $$OUT_PWD/../$$getConfigDir()/gen
INCLUDEPATH += $$QTWEBENGINE_ROOT/src/pdf \
               $$CHROMIUM_GEN_DIR \
               $$CHROMIUM_SRC_DIR \
               api

DEFINES += QT_BUILD_PDF_LIB
win32: DEFINES += NOMINMAX

QMAKE_DOCS = $$PWD/doc/qtpdf.qdocconf

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON += -wd"4100"
}

include($${QTWEBENGINE_ROOT}/src/buildtools/config/linking.pri)

# install static dependencies and handle prl files for static builds

static:!isEmpty(NINJA_ARCHIVES) {
    static_dep_pri = $$OUT_PWD/$$getConfigDir()/$${TARGET}_static_dep.pri
    !include($${static_dep_pri}) {
        error("Could not find the prl information.")
    }
    ninja_archives = $$eval($$list($$NINJA_ARCHIVES))
    ninja_archs_install.files = $${ninja_archives}
    ninja_archs_install.path = $$[QT_INSTALL_LIBS]/static_chrome
    ninja_archs_install.CONFIG = no_check_exist
    INSTALLS += ninja_archs_install
}

SOURCES += \
    qpdfbookmarkmodel.cpp \
    qpdfdestination.cpp \
    qpdfdocument.cpp \
    qpdflinkmodel.cpp \
    qpdfpagenavigation.cpp \
    qpdfpagerenderer.cpp \
    qpdfsearchmodel.cpp \
    qpdfsearchresult.cpp \
    qpdfselection.cpp \

# all "public" headers must be in "api" for sync script and to hide auto generated headers
# by Chromium in case of in-source build

HEADERS += \
    api/qpdfbookmarkmodel.h \
    api/qpdfdestination.h \
    api/qpdfdestination_p.h \
    api/qpdfdocument.h \
    api/qpdfdocument_p.h \
    api/qpdfdocumentrenderoptions.h \
    api/qtpdfglobal.h \
    api/qpdflinkmodel_p.h \
    api/qpdflinkmodel_p_p.h \
    api/qpdfnamespace.h \
    api/qpdfpagenavigation.h \
    api/qpdfpagerenderer.h \
    api/qpdfsearchmodel.h \
    api/qpdfsearchmodel_p.h \
    api/qpdfsearchresult.h \
    api/qpdfsearchresult_p.h \
    api/qpdfselection.h \
    api/qpdfselection_p.h \

load(qt_module)
