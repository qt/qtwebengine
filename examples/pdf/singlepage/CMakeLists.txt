# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(singlepage LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/pdf/singlepage")

find_package(Qt6 REQUIRED COMPONENTS Gui Qml OPTIONAL_COMPONENTS Svg)
if(NOT TARGET Qt::Svg)
    message(WARNING "QtSvg is required as runtime dependency for qtpdfquick examples.")
endif()

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(singlepage
    main.cpp
)

qt_add_qml_module(singlepage
    URI SinglePageModule
    QML_FILES
        Viewer.qml
    RESOURCES
        resources/document-open.svg
        resources/edit-clear.svg
        resources/edit-copy.svg
        resources/edit-select-all.svg
        resources/go-down-search.svg
        resources/go-next-view-page.svg
        resources/go-previous-view-page.svg
        resources/go-up-search.svg
        resources/rotate-left.svg
        resources/rotate-right.svg
        resources/zoom-fit-best.svg
        resources/zoom-fit-width.svg
        resources/zoom-in.svg
        resources/zoom-original.svg
        resources/zoom-out.svg

        resources/test.pdf
)

set_target_properties(singlepage PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(singlepage PUBLIC
    Qt::Gui
    Qt::Qml
)

install(TARGETS singlepage
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
