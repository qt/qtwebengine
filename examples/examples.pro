
include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginequick/qtwebenginequick-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginewidgets/qtwebenginewidgets-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdfwidgets/qtpdfwidgets-config.pri)

QT_FOR_CONFIG += \
     buildtools-private \
     webenginequick-private \
     webenginewidgets-private \
     pdf-private \
     pdfwidgets-private

TEMPLATE=subdirs

qtConfig(build-qtwebengine-core):qtConfig(webengine-core-support) {
    qtConfig(webengine-quick): SUBDIRS += webengine
    qtConfig(webengine-widgets): SUBDIRS += webenginewidgets
}

qtConfig(build-qtpdf):qtConfig(webengine-qtpdf-support) {
    SUBDIRS += pdf
    qtConfig(pdf-widgets): SUBDIRS += pdfwidgets
}
