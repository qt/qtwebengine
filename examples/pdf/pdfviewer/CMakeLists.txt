# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(pdfviewer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/pdf/pdfviewer")

find_package(Qt6 REQUIRED COMPONENTS Gui Qml)

qt_add_executable(pdfviewerquick
    main.cpp
)

set_target_properties(pdfviewerquick PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(pdfviewerquick PUBLIC
    Qt::Gui
    Qt::Qml
)

set(viewer_resource_files
    "resources/document-open.svg"
    "resources/edit-clear.svg"
    "resources/edit-copy.svg"
    "resources/edit-select-all.svg"
    "resources/go-down-search.svg"
    "resources/go-next-view-page.svg"
    "resources/go-previous-view-page.svg"
    "resources/go-up-search.svg"
    "resources/rotate-left.svg"
    "resources/rotate-right.svg"
    "resources/test.pdf"
    "resources/zoom-fit-best.svg"
    "resources/zoom-fit-width.svg"
    "resources/zoom-in.svg"
    "resources/zoom-original.svg"
    "resources/zoom-out.svg"
    "viewer.qml"
)

qt_add_resources(pdfviewerquick "viewer"
    PREFIX
        "/pdfviewer"
    FILES
        ${viewer_resource_files}
)

install(TARGETS pdfviewerquick
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
