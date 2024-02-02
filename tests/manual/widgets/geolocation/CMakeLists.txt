# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(geolocation LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(geolocation
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Test
        Qt::WebEngineWidgets
    ENABLE_AUTOGEN_TOOLS
        moc
)

set_target_properties(geolocation PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.dev.webenginewidgets.geolocation"
)

set(geolocation_resource_files
    "geolocation.html"
)

qt_add_resources(geolocation "geolocation"
    PREFIX
    "/"
    FILES
    ${geolocation_resource_files}
)

    foreach(permission_plugin IN LISTS QT_ALL_PLUGINS_FOUND_BY_FIND_PACKAGE_permissions)
        set(permission_plugin "${QT_CMAKE_EXPORT_NAMESPACE}::${permission_plugin}")
        qt6_import_plugins(geolocation INCLUDE ${permission_plugin})
    endforeach()

if (APPLE)
    set_target_properties(geolocation PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Need to sign application for location permissions to work
        add_custom_command(TARGET geolocation
            POST_BUILD COMMAND codesign -s - geolocation.app)
    endif()
endif()
