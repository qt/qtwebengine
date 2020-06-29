
include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webengine/qtwebengine-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginewidgets/qtwebenginewidgets-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdfwidgets/qtpdfwidgets-config.pri)

QT_FOR_CONFIG += \
    buildtools-private \
    webengine-private \
    webenginewidgets-private \
    pdf-private \
    pdfwidgets-private

TEMPLATE = subdirs

qtConfig(build-qtwebengine-core):qtConfig(webengine-core-support) {
    qtConfig(webengine-qml): SUBDIRS += quick
    qtConfig(webengine-widgets): SUBDIRS += core widgets
}

qtConfig(build-qtpdf):qtConfig(webengine-qtpdf-support) {
    SUBDIRS += pdf
}

