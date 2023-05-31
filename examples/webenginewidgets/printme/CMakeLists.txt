# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(printme LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/printme")

find_package(Qt6 REQUIRED COMPONENTS Core Gui PrintSupport WebEngineWidgets)

qt_add_executable(printme
    main.cpp
    printhandler.cpp printhandler.h
)

set_target_properties(printme PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(printme PUBLIC
    Qt::Core
    Qt::Gui
    Qt::PrintSupport
    Qt::WebEngineWidgets
)

# Resources:
set(data_resource_files
    "data/icon.svg"
    "data/index.html"
    "data/style.css"
)

qt_add_resources(printme "data"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${data_resource_files}
)

install(TARGETS printme
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
