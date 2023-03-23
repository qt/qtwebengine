# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(recipebrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/recipebrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(recipebrowser
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    stylesheetdialog.cpp stylesheetdialog.h stylesheetdialog.ui
    document.cpp document.h
)

set_target_properties(recipebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(recipebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(recipebrowser_resource_files
    "assets/3rdparty/markdown.css"
    "assets/3rdparty/marked.js"
    "assets/custom.css"
    "assets/custom.js"
    "assets/pages/burger.html"
    "assets/pages/cupcakes.html"
    "assets/pages/images/burger.jpg"
    "assets/pages/images/cupcakes.jpg"
    "assets/pages/images/pasta.jpg"
    "assets/pages/images/pizza.jpg"
    "assets/pages/images/skewers.jpg"
    "assets/pages/images/soup.jpg"
    "assets/pages/images/steak.jpg"
    "assets/pages/pasta.html"
    "assets/pages/pizza.html"
    "assets/pages/skewers.html"
    "assets/pages/soup.html"
    "assets/pages/steak.html"
)

qt_add_resources(recipebrowser "recipebrowser"
    PREFIX
        "/"
    BASE
        "assets"
    FILES
        ${recipebrowser_resource_files}
)

install(TARGETS recipebrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
