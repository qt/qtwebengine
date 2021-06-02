qt_internal_add_qml_module(qtwebenginetestsupportplugin
    URI "QtWebEngine.testsupport"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtWebEngineTestSupportPlugin
    PLUGIN_TARGET qtwebenginetestsupportplugin
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::GuiPrivate
        Qt::Qml
        Qt::WebEngineQuickPrivate
)
make_install_only(qtwebenginetestsupportplugin)
