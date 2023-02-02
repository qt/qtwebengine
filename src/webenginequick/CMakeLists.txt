# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(ui)

qt_internal_add_qml_module(WebEngineQuick
    URI "QtWebEngine"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtWebEnginePlugin
    DEPENDENCIES
        QtQuick/auto
        QtWebChannel/auto
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
        api/qquickwebenginenewwindowrequest.cpp api/qquickwebenginenewwindowrequest_p.h
        api/qquickwebengineprofile.cpp api/qquickwebengineprofile.h api/qquickwebengineprofile_p.h
        api/qquickwebenginescriptcollection.cpp api/qquickwebenginescriptcollection_p.h api/qquickwebenginescriptcollection_p_p.h
        api/qquickwebenginesettings.cpp api/qquickwebenginesettings_p.h
        api/qquickwebenginesingleton.cpp api/qquickwebenginesingleton_p.h
        api/qquickwebenginetouchhandleprovider.cpp
        api/qquickwebenginetouchhandleprovider_p_p.h
        api/qquickwebenginetouchhandle.cpp api/qquickwebenginetouchhandle_p.h
        api/qquickwebenginetouchselectionmenurequest.cpp
        api/qquickwebenginetouchselectionmenurequest_p.h
        api/qquickwebenginetouchselectionmenurequest_p_p.h
        api/qquickwebengineview.cpp api/qquickwebengineview_p.h
        api/qquickwebengineview_p_p.h
        api/qquickwebengineforeigntypes_p.h
        api/qtwebenginequickglobal.cpp api/qtwebenginequickglobal.h
        api/qtwebenginequickglobal_p.h
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

qt_internal_extend_target(WebEngineQuick CONDITION QT_FEATURE_webengine_webchannel
    PUBLIC_LIBRARIES
        Qt::WebChannelQuick
)

qt_internal_extend_target(WebEngineQuick CONDITION QT_FEATURE_accessibility
    SOURCES
        qquickwebengine_accessible.cpp qquickwebengine_accessible.h
)

qt_internal_extend_target(qtwebenginequickplugin
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
)

