CXX_MODULE = qml
TARGET  = pdfplugin
TARGETPATH = QtQuick/Pdf
IMPORT_VERSION = 1.0

# qpdfdocument_p.h includes pdfium headers which we must find in order to use private API
CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
INCLUDEPATH += $$CHROMIUM_SRC_DIR

#QMAKE_DOCS = $$PWD/doc/qtquickpdf.qdocconf

PDF_QML_FILES = \
    qml/PdfMultiPageView.qml \
    qml/PdfPageView.qml \
    qml/PdfScrollablePageView.qml \

QML_FILES += $$PDF_QML_FILES qmldir

RESOURCES += resources.qrc

SOURCES += \
    plugin.cpp \
    qquickpdfdocument.cpp \
    qquickpdflinkmodel.cpp \
    qquickpdfnavigationstack.cpp \
    qquickpdfsearchmodel.cpp \
    qquickpdfselection.cpp \
    qquicktableviewextra.cpp \

HEADERS += \
    qquickpdfdocument_p.h \
    qquickpdflinkmodel_p.h \
    qquickpdfnavigationstack_p.h \
    qquickpdfsearchmodel_p.h \
    qquickpdfselection_p.h \
    qquicktableviewextra_p.h \

QT += pdf pdf-private gui core qml quick quick-private
include($${OUT_PWD}/../$$getConfigDir()/QtPdf_static_dep.pri)
load(qml_plugin)
