# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.19)
    project(webrtc LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_internal_add_manual_test(webrtc
    GUI
    SOURCES
        main.cpp
        mediaPicker.ui
        qrc.qrc
    LIBRARIES
        Qt::Core
        Qt::HttpServer
        Qt::Gui
        Qt::WebEngineWidgets
)
