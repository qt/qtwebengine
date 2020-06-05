load(functions)

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webengine/qtwebengine-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/webenginewidgets/qtwebenginewidgets-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdfwidgets/qtpdfwidgets-config.pri)

QT_FOR_CONFIG += \
    buildtools-private \
    webenginecore \
    webenginecore-private \
    webengine-private \
    webenginewidgets-private \
    pdf-private \
    pdfwidgets-private

TEMPLATE = subdirs

qtConfig(build-qtwebengine-core):qtConfig(webengine-core-support) {
    core.depends = buildtools
    process.depends = core
    webengine.depends = core
    webenginewidgets.depends = core webengine
    webengine_plugin.subdir = webengine/plugin
    webengine_plugin.target = sub-webengine-plugin
    webengine_plugin.depends = webengine

    SUBDIRS += buildtools core process

    qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
        SUBDIRS += qwebengine_convert_dict
        qwebengine_convert_dict.subdir = tools/qwebengine_convert_dict
        qwebengine_convert_dict.depends = core
    }

    qtConfig(webengine-qml) {
        SUBDIRS += webengine
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

