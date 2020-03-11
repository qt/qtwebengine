CXX_MODULE = qml
TARGET  = pdfplugin
TARGETPATH = QtQuick/Pdf
IMPORT_VERSION = 1.0

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

HEADERS += \
    qquickpdfdocument_p.h \
    qquickpdflinkmodel_p.h \
    qquickpdfnavigationstack_p.h \
    qquickpdfsearchmodel_p.h \
    qquickpdfselection_p.h \

QT += pdf quick-private gui gui-private core core-private qml qml-private

load(qml_plugin)
