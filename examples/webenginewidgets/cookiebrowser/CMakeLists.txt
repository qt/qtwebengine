# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(cookiebrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/cookiebrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(cookiebrowser
    cookiedialog.ui
    cookiewidget.ui
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

set_target_properties(cookiebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(cookiebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(cookiebrowser_resource_files
    "3rdparty/view-refresh.png"
)

qt_add_resources(cookiebrowser "cookiebrowser"
    PREFIX
        "/"
    BASE
        "3rdparty"
    FILES
        ${cookiebrowser_resource_files}
)

install(TARGETS cookiebrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
