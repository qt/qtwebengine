# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../../util/util.cmake)

qt_internal_add_test(tst_webenginedriver
    SOURCES
        ../tst_webenginedriver.cpp
    LIBRARIES
        Qt::Network
        Qt::WebEngineCore
        Qt::WebEngineWidgets
        Test::Util
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

set(tst_webenginedriver_resource_files
    "../resources/input.html"
)

qt_internal_add_resource(tst_webenginedriver "tst_webenginedriver"
    PREFIX
        "/"
    FILES
        ${tst_webenginedriver_resource_files}
)
