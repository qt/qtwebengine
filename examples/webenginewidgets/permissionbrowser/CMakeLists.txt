# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.5)
project(permissionbrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/permissionbrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(permissionbrowser
    permissiondialog.ui
    permissionwidget.ui
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

if(WIN32)
    set_property(
        TARGET permissionbrowser
        APPEND PROPERTY
            SOURCES permissionbrowser.exe.manifest)
endif()

set_target_properties(permissionbrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.webenginewidgets.permissionbrowser"
)

target_link_libraries(permissionbrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(permissionbrowser_resource_files
    "resources/3rdparty/view-refresh.png"
    "resources/3rdparty/go-next.png"
    "resources/3rdparty/go-previous.png"
    "resources/AppLogoColor.png"
    "resources/landing.html"
)

qt_add_resources(permissionbrowser "permissionbrowser"
    PREFIX
        "/"
    BASE
        "resources"
    FILES
        ${permissionbrowser_resource_files}
)

if (APPLE)
    set_target_properties(permissionbrowser PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.cmake.macos.plist"
    )

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Need to sign application for location permissions to work
        if(QT_FEATURE_debug_and_release)
            set(exe_path "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/")
        else()
            unset(exe_path)
        endif()
        add_custom_command(TARGET permissionbrowser
            POST_BUILD COMMAND codesign --force -s - ${exe_path}permissionbrowser.app
        )
    endif()
endif()

install(TARGETS permissionbrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
