# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
     cmake_minimum_required(VERSION 3.16)
     project(minimal LANGUAGES CXX)
     find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)
endif()

qt_internal_add_manual_test(webengine-minimal-qml
    SOURCES
        main.cpp
)

set_target_properties(webengine-minimal-qml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(webengine-minimal-qml PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_add_resources(webengine-minimal-qml "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
