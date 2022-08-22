# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(contentmanipulation LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/contentmanipulation")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(contentmanipulation
    main.cpp
    mainwindow.cpp mainwindow.h
)

set_target_properties(contentmanipulation PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(contentmanipulation PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(jquery_resource_files
    "jquery.min.js"
)

qt_add_resources(contentmanipulation "jquery"
    PREFIX
        "/"
    FILES
        ${jquery_resource_files}
)

install(TARGETS contentmanipulation
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
