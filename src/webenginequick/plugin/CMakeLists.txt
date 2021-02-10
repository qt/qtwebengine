qt_internal_add_qml_module(qtwebengineplugin
    URI "QtWebEngine"
    VERSION "${PROJECT_VERSION}"
    CLASSNAME QtWebEnginePlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuickPrivate
)
make_install_only(qtwebengineplugin)
