qt_internal_add_plugin(QWebEngineViewPlugin
    OUTPUT_NAME qwebengineview
    PLUGIN_TYPE designer
    SOURCES
        qwebengineview_plugin.cpp qwebengineview_plugin.h
    LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::WebEngineWidgets
)

set(qwebengineview_plugin_resource_files
    "images/qwebengineview.png"
)

qt_internal_add_resource(QWebEngineViewPlugin "qwebengineview_plugin"
    PREFIX
        "/qt-project.org/qwebengineview"
    FILES
        ${qwebengineview_plugin_resource_files}
)
