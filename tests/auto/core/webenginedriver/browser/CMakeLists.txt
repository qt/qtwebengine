# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_executable(testbrowser
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WebEngineWidgets
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/..
)
