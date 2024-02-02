# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.19)
    project(touchbrowser LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_AUTORCC ON)
set(TOUCHMOCKING_DIR "../../touchmocking")

include_directories(${TOUCHMOCKING_DIR})
add_definitions(-DQUICK_TOUCHBROWSER)

qt_internal_add_manual_test(touchbrowser-quick
    GUI
    SOURCES
        main.cpp
        resources.qrc
        ${TOUCHMOCKING_DIR}/touchmockingapplication.cpp
        ${TOUCHMOCKING_DIR}/touchmockingapplication.h
        ${TOUCHMOCKING_DIR}/utils.h
    LIBRARIES
        Qt::Core
        Qt::Quick
        Qt::WebEngineQuick
    ENABLE_AUTOGEN_TOOLS
        moc
)

