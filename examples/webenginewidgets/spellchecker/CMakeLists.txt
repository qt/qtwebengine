# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(spellchecker LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/spellchecker")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(spellchecker
    main.cpp
    webview.cpp webview.h
)

set_target_properties(spellchecker PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(spellchecker PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(spellchecker_resource_files
    "data/icon.svg"
    "data/index.html"
    "data/style.css"
)

qt_add_resources(spellchecker "spellchecker"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${spellchecker_resource_files}
)

install(TARGETS spellchecker
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

file(GLOB_RECURSE dicts
    ABSOLUTE ${CMAKE_CURRENT_LIST_DIR}/dict
    */*.dic
)

foreach(dictFile ${dicts})
    qt_add_webengine_dictionary(
        TARGET spellchecker
        SOURCE "${dictFile}"
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
