find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Gui Qml Quick)
find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS PrintSupport WebChannel Positioning Test)

get_configure_mode(configureMode)


if(QT_FEATURE_webenginequick_testsupport)
    add_subdirectory(testsupport)
endif()
add_subdirectory(ui)

qt_internal_add_qml_module(WebEngineQuick
    ${configureMode}
    URI "QtWebEngine"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtWebEnginePlugin
    PLUGIN_TARGET qtwebenginequickplugin
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    SOURCES
        api/qquickwebengineaction.cpp api/qquickwebengineaction_p.h
        api/qquickwebengineaction_p_p.h
        api/qquickwebengineclientcertificateselection.cpp api/qquickwebengineclientcertificateselection_p.h
        api/qquickwebenginedialogrequests.cpp api/qquickwebenginedialogrequests_p.h
        api/qquickwebenginedownloadrequest.cpp api/qquickwebenginedownloadrequest_p.h
        api/qquickwebenginefaviconprovider.cpp
        api/qquickwebenginefaviconprovider_p_p.h
        api/qquickwebenginenewviewrequest.cpp api/qquickwebenginenewviewrequest_p.h
        api/qquickwebengineprofile.cpp api/qquickwebengineprofile.h api/qquickwebengineprofile_p.h
        api/qquickwebenginescriptcollection.cpp api/qquickwebenginescriptcollection.h
        api/qquickwebenginesettings.cpp api/qquickwebenginesettings_p.h
        api/qquickwebenginesingleton.cpp api/qquickwebenginesingleton_p.h
        api/qquickwebenginetouchhandleprovider.cpp
        api/qquickwebenginetouchhandleprovider_p_p.h
        api/qquickwebengineview.cpp api/qquickwebengineview_p.h
        api/qquickwebengineview_p_p.h
        api/qquickwebengineforeigntypes_p.h
        api/qtwebenginequickglobal.cpp api/qtwebenginequickglobal.h
        api/qtwebenginequickglobal_p.h
        render_widget_host_view_qt_delegate_quick.cpp render_widget_host_view_qt_delegate_quick.h
        render_widget_host_view_qt_delegate_quickwindow.cpp render_widget_host_view_qt_delegate_quickwindow.h
        ui_delegates_manager.cpp ui_delegates_manager.h
    DEFINES
        QT_BUILD_WEBENGINE_LIB
    INCLUDE_DIRECTORIES
        ../core
        api
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::WebEngineCorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::WebEngineCore
)

qt_internal_extend_target(qtwebenginequickplugin
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
)

make_install_only(WebEngineQuick)
make_install_only(qtwebenginequickplugin)

qt_internal_extend_target(WebEngineQuick CONDITION QT_FEATURE_webenginequick_testsupport
    LIBRARIES
       Qt::WebEngineTestSupportPrivate
)
