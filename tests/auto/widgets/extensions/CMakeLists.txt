# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)
include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_qwebengineextension
    SOURCES
        tst_qwebengineextension.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
        Test::HttpServer
)

set(tst_qwebengineextension_resource_files
    "resources/index.html"
)

qt_internal_add_resource(tst_qwebengineextension "tst_qwebengineextension"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineextension_resource_files}
)
