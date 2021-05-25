find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Gui Widgets Quick QuickWidgets)
find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS PrintSupport WebChannel Positioning)

get_configure_mode(configureMode)

qt_internal_add_module(WebEngineWidgets
    ${configureMode}
    SOURCES
        api/qtwebenginewidgetsglobal.h
        api/qwebenginenotificationpresenter.cpp api/qwebenginenotificationpresenter_p.h
        api/qwebengineview.cpp api/qwebengineview.h api/qwebengineview_p.h
        render_widget_host_view_qt_delegate_widget.cpp render_widget_host_view_qt_delegate_widget.h
    DEFINES
        QT_BUILD_WEBENGINEWIDGETS_LIB
    INCLUDE_DIRECTORIES
        ../core
        api
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickWidgets
        Qt::WidgetsPrivate
        Qt::WebEngineCorePrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::WebEngineCore
)
make_install_only(WebEngineWidgets)
