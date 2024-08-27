# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quicknanobrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/quicknanobrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebEngineQuick)

qt_add_executable(quicknanobrowser
    main.cpp
    utils.h
)

if(WIN32)
    set_property(
        TARGET quicknanobrowser
        APPEND PROPERTY
            SOURCES quicknanobrowser.exe.manifest)
endif()

set_target_properties(quicknanobrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.webenginequick.quicknanobrowser"
)

target_link_libraries(quicknanobrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::WebEngineQuick
)

qt_add_qml_module(quicknanobrowser
    URI BrowserUtils
    VERSION 1.0
    RESOURCE_PREFIX /
)

# Resources:
set(resources_resource_files
    "ApplicationRoot.qml"
    "BrowserDialog.qml"
    "BrowserWindow.qml"
    "DownloadView.qml"
    "FindBar.qml"
    "FullScreenNotification.qml"
    "WebAuthDialog.qml"
)

qt_add_resources(quicknanobrowser "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

set(resources1_resource_files
    "icons/3rdparty/go-next.png"
    "icons/3rdparty/go-previous.png"
    "icons/3rdparty/process-stop.png"
    "icons/3rdparty/view-refresh.png"
)

qt_add_resources(quicknanobrowser "resources1"
    PREFIX
        "/icons"
    BASE
        "icons/3rdparty"
    FILES
        ${resources1_resource_files}
)

if(TARGET Qt::Widgets)
    target_link_libraries(quicknanobrowser PUBLIC
        Qt::Widgets
    )
endif()

if (APPLE)
    set_target_properties(quicknanobrowser PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.cmake.macos.plist"
    )

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Need to sign application for location permissions to work
        if(QT_FEATURE_debug_and_release)
            set(exe_path "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/")
        else()
            unset(exe_path)
        endif()
        add_custom_command(TARGET quicknanobrowser
            POST_BUILD COMMAND codesign --force -s - ${exe_path}quicknanobrowser.app
        )
    endif()
endif()

install(TARGETS quicknanobrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
