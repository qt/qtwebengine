# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(touchbrowser LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

if(NOT CMAKE_CROSSCOMPILING)
    add_definitions(-DDESKTOP_BUILD)
    set(SOURCES "touchmockingapplication.cpp")
endif()

qt_internal_add_manual_test(touchbrowser
    GUI
    SOURCES
        main.cpp
        utils.h
        ${SOURCES}
    LIBRARIES
        Qt::GuiPrivate
        Qt::Quick
        Qt::WebEngineQuick
    ENABLE_AUTOGEN_TOOLS
        moc
)

set(touchbrowser_resource_files
    "AddressBar.qml"
    "main.qml"
    "MockTouchPoint.qml"
    "touchpoint.png"
)

qt_add_resources(touchbrowser "touchbrowser"
    PREFIX
    "/"
    FILES
    ${touchbrowser_resource_files}
)
