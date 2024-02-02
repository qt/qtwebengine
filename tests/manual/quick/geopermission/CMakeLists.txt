# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_geopermission LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_geopermission
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::WebEngineQuick
)

if(WIN32)
    set_property(
        TARGET tst_geopermission
        APPEND PROPERTY
            SOURCES tst_geopermission.exe.manifest)
endif()

set_target_properties(tst_geopermission PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.dev.webenginequick.tst_geopermission"
)

# Resources:
set(resources_resource_files
    "tst_geopermission.qml"
    "geolocation.html"
)

qt_add_resources(tst_geopermission "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

foreach(permission_plugin IN LISTS QT_ALL_PLUGINS_FOUND_BY_FIND_PACKAGE_permissions)
    set(permission_plugin "${QT_CMAKE_EXPORT_NAMESPACE}::${permission_plugin}")
        qt6_import_plugins(tst_geopermission INCLUDE ${permission_plugin})
endforeach()

if (APPLE)
    set_target_properties(tst_geopermission PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Need to sign application for location permissions to work
        add_custom_command(TARGET tst_geopermission
            POST_BUILD COMMAND codesign -s - tst_geopermission.app)
    endif()
endif()

