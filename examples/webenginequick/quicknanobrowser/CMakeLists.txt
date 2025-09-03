# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quicknanobrowser LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebEngineQuick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(quicknanobrowser
    main.cpp
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
    QML_FILES
        ApplicationRoot.qml
        BrowserDialog.qml
        BrowserWindow.qml
        DownloadView.qml
        FindBar.qml
        FullScreenNotification.qml
        WebAuthDialog.qml
    SOURCES
        utils.h
    RESOURCES
        icons/3rdparty/go-next.png
        icons/3rdparty/go-previous.png
        icons/3rdparty/process-stop.png
        icons/3rdparty/view-refresh.png
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
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET quicknanobrowser
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
