# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(inputmethods LANGUAGES CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(inputmethods
    GUI
    SOURCES
        colorpicker.cpp
        controlview.cpp
        main.cpp
        referenceview.cpp
        testview.cpp
        webview.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Test
        Qt::WebEngineWidgets
    ENABLE_AUTOGEN_TOOLS
        moc
)

set(inputmethods_resource_files
    "testdata.csv"
)

qt_add_resources(inputmethods "inputmethods"
    PREFIX
    "/"
    FILES
    ${inputmethods_resource_files}
)
