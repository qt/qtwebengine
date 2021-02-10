load(functions)

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginequick/qtwebenginequick-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginewidgets/qtwebenginewidgets-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdfwidgets/qtpdfwidgets-config.pri)

QT_FOR_CONFIG += \
    buildtools-private \
    webenginecore \
    webenginecore-private \
    webenginequick \
    webenginequick-private \
    webenginewidgets \
    webenginewidgets-private \
    pdf-private \
    pdfwidgets-private

TEMPLATE = subdirs

qtConfig(build-qtwebengine-core):qtConfig(webengine-core-support) {
    core.depends = buildtools
    process.depends = core
    webenginequick.depends = core
    webenginewidgets.depends = core webenginequick
    webenginequick_plugin.subdir = webenginequick/plugin
    webenginequick_plugin.target = sub-webenginequick-plugin
    webenginequick_plugin.depends = webenginequick

    SUBDIRS += buildtools core process

    qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
        SUBDIRS += qwebengine_convert_dict
        qwebengine_convert_dict.subdir = tools/qwebengine_convert_dict
        qwebengine_convert_dict.depends = core
    }

    qtConfig(webengine-quick) {
        SUBDIRS += webenginequick
    }

    qtConfig(webengine-widgets) {
        SUBDIRS += plugins webenginewidgets
        plugins.depends = webenginewidgets
    }
}

qtConfig(build-qtpdf):qtConfig(webengine-qtpdf-support) {
    pdf.depends = buildtools
    qtConfig(build-qtwebengine-core):qtConfig(webengine-core-support): pdf.depends += core
    !contains(SUBDIRS, buildtools): SUBDIRS += buildtools
    !contains(SUBDIRS, plugins): SUBDIRS += plugins
    SUBDIRS += pdf
    plugins.depends += pdf
    qtConfig(pdf-widgets) {
        pdfwidgets.depends = pdf
        SUBDIRS += pdfwidgets
    }
}

# this needs to be last line for qmake -r
qtConfig(build-qtwebengine-core):!contains(SUBDIRS, core): SUBDIRS += core
qtConfig(build-qtpdf):!contains(SUBDIRS, pdf): SUBDIRS += pdf
