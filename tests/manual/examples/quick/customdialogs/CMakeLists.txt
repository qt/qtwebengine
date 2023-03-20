# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(customdialogs LANGUAGES CXX)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)
endif()

qt_internal_add_manual_test(customdialogs
    SOURCES main.cpp server.cpp server.h
)

set_target_properties(customdialogs PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(customdialogs PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

set(customdialogs_resource_files
    "MessageRectangle.qml"
    "SwitchButton.qml"
    "WebView.qml"
    "forms/Authentication.qml"
    "forms/AuthenticationForm.ui.qml"
    "forms/ColorCell.qml"
    "forms/ColorPicker.qml"
    "forms/ColorPickerForm.ui.qml"
    "forms/CustomButton.qml"
    "forms/FilePicker.qml"
    "forms/FilePickerForm.ui.qml"
    "forms/FileRow.qml"
    "forms/JavaScript.qml"
    "forms/JavaScriptForm.ui.qml"
    "forms/Menu.qml"
    "forms/MenuForm.ui.qml"
    "forms/TouchSelectionMenu.qml"
    "forms/TouchSelectionMenuForm.ui.qml"
    "icon.svg"
    "index.html"
    "main.qml"
    "style.css"
)

qt_add_resources(customdialogs "customdialogs"
    PREFIX
        "/"
    FILES
        ${customdialogs_resource_files}
)

