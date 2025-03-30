# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtwebengine_cmake_tests)

enable_testing()

find_package(Qt6Core REQUIRED)

include("${_Qt6CTestMacros}")

set(module_includes "")
if(TARGET Qt6::WebEngineCore)
    list(APPEND module_includes
        WebEngineCore QWebEnginePage
    )
endif()
if(TARGET Qt6::WebEngineWidgets)
    list(APPEND module_includes
        WebEngineWidgets QWebEngineView
    )
endif()
if(TARGET Qt6::Pdf)
    list(APPEND module_includes
        Pdf QPdfDocument
    )
endif()

_qt_internal_test_module_includes(${module_includes})
