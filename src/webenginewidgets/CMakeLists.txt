# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Designer)
    add_subdirectory(plugins/qwebengineview)
else()
    message(WARNING "Qt tools Designer is required to compile designer plugin")
endif()
qt_internal_add_module(WebEngineWidgets
    SOURCES
        api/qtwebenginewidgetsglobal.h
        api/qwebenginenotificationpresenter.cpp api/qwebenginenotificationpresenter_p.h
        api/qwebengineview.cpp api/qwebengineview.h api/qwebengineview_p.h
        ui/autofillpopupwidget.cpp ui/autofillpopupwidget_p.h
        ui/touchhandlewidget.cpp ui/touchhandlewidget_p.h
        ui/touchselectionmenuwidget.cpp ui/touchselectionmenuwidget_p.h
    DEFINES
        QT_BUILD_WEBENGINEWIDGETS_LIB
    INCLUDE_DIRECTORIES
        ../core
        api
        ui
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

qt_internal_extend_target(WebEngineWidgets CONDITION QT_FEATURE_accessibility
    SOURCES
        qwebengine_accessible.cpp qwebengine_accessible.h
)

qt_internal_extend_target(WebEngineWidgets CONDITION QT_FEATURE_webengine_printing_and_pdf
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)
