CXX_MODULE = qml
TARGET  = pdfplugin
TARGETPATH = QtQuick/Pdf
IMPORT_VERSION = 1.0

#QMAKE_DOCS = $$PWD/doc/qtquickpdf.qdocconf

SOURCES += \
    qquickpdfdocument.cpp \
    plugin.cpp

HEADERS += \
    qquickpdfdocument_p.h

QT += pdf quick-private gui gui-private core core-private qml qml-private

load(qml_plugin)
