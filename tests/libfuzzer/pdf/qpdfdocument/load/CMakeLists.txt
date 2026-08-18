# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(load LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Pdf)

qt_add_executable(load
    main.cpp
)

target_link_libraries(load PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Pdf
)
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    target_link_libraries(load PRIVATE
        $ENV{LIB_FUZZING_ENGINE}
    )
else()
    target_link_libraries(load PRIVATE
        -fsanitize=fuzzer
    )
endif()

qt_import_plugins(load
    INCLUDE Qt::QMinimalIntegrationPlugin
)
