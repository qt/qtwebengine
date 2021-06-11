qt_internal_add_qml_module(WebEngineTestSupport
    ${configureMode}
    URI "QtWebEngine.testsupport"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtWebEngineTestSupportPlugin
    PLUGIN_TARGET qtwebenginetestsupportplugin
    NO_PLUGIN_OPTIONAL
    SOURCES
        qquickwebenginetestsupport.cpp qquickwebenginetestsupport_p.h
    DEFINES
        QT_BUILD_WEBENGINEQUICK_TESTSUPPORT_LIB
    LIBRARIES
        Qt::GuiPrivate
        Qt::Qml
        Qt::TestPrivate
        Qt::WebEngineCore
)
make_install_only(WebEngineTestSupport)
make_install_only(qtwebenginetestsupportplugin)
