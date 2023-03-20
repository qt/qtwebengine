# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(customtouchhandle LANGUAGES CXX)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)
endif()

qt_internal_add_manual_test(customtouchhandle
    SOURCES main.cpp
)

set_target_properties(customtouchhandle PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(customtouchhandle PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(customtouchhandle "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

