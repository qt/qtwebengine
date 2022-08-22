# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(notifications LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/notifications")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(notifications
    main.cpp
    notificationpopup.h
)

set_target_properties(notifications PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(notifications PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(data_resource_files
    "data/icon.png"
    "data/index.html"
)

qt_add_resources(notifications "data"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${data_resource_files}
)

install(TARGETS notifications
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
