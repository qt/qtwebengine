qt_internal_add_qml_module(qtwebengineplugin
    URI "QtWebEngine"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtWebEnginePlugin
    PLUGIN_TARGET qtwebengineplugin
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuickPrivate
)
make_install_only(qtwebengineplugin)
