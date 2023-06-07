# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(lifecycle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/lifecycle")

find_package(Qt6 REQUIRED COMPONENTS Core Gui QuickControls2 WebEngineQuick)

qt_add_executable(lifecycle
    main.cpp
    utils.h
)

set_target_properties(lifecycle PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(lifecycle PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(resources_resource_files
    "WebBrowser.qml"
    "WebTab.qml"
    "WebTabBar.qml"
    "WebTabButton.qml"
    "WebTabStack.qml"
    "WebToolButton.qml"
    "qtquickcontrols2.conf"
)

qt_add_resources(lifecycle "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

install(TARGETS lifecycle
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
