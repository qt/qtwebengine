load(platform)

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private

TEMPLATE = subdirs

isWebEngineCoreBuild():qtConfig(webengine-core) {
    include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
    include($$QTWEBENGINE_OUT_ROOT/src/webengine/qtwebengine-config.pri)
    include($$QTWEBENGINE_OUT_ROOT/src/webenginewidgets/qtwebenginewidgets-config.pri)
    QT_FOR_CONFIG += webenginecore webenginecore-private webengine-private webenginewidgets-private

    process.depends = core
    webengine.depends = core
    webenginewidgets.depends = core webengine
    webengine_plugin.subdir = webengine/plugin
    webengine_plugin.target = sub-webengine-plugin
    webengine_plugin.depends = webengine

    core.depends = buildtools

    SUBDIRS += buildtools \
        core \
        process

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
} else {
    !isEmpty(skipBuildReason):!build_pass {
            log(QtWebEngine will not be built. $${skipBuildReason} $${EOL})
            errorbuild.commands = @echo QtWebEngine will not be built. $${skipBuildReason}
            errorbuild.CONFIG = phony
            QMAKE_EXTRA_TARGETS += errorbuild
            first.depends += errorbuild
            QMAKE_EXTRA_TARGETS += first
   }
}
