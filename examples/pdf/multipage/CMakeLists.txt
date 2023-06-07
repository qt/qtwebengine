# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(multipage LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/pdf/multipage")

find_package(Qt6 REQUIRED COMPONENTS Gui Qml)

qt_add_executable(multipage
    main.cpp
    pdfapplication.h
    pdfapplication.cpp
)

if (APPLE AND NOT IOS)
    set(MACOSX_BUNDLE_ICON_FILE multipage.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/multipage.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(multipage PRIVATE ${app_icon_macos})
    set(MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist")
endif()

set_target_properties(multipage PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(multipage PUBLIC
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
    "resources/sidebar-collapse-left.svg"
    "resources/sidebar-expand-left.svg"
    "resources/test.pdf"
    "resources/zoom-fit-best.svg"
    "resources/zoom-fit-width.svg"
    "resources/zoom-in.svg"
    "resources/zoom-original.svg"
    "resources/zoom-out.svg"
    "viewer.qml"
)

qt_add_resources(multipage "viewer"
    PREFIX
        "/pdfviewer"
    FILES
        ${viewer_resource_files}
)

install(TARGETS multipage
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
