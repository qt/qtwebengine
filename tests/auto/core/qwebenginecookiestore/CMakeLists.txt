# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginecookiestore
    SOURCES
        tst_qwebenginecookiestore.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Test::Util
)

set(tst_qwebenginecookiestore_resource_files
    "resources/content.html"
    "resources/index.html"
)

qt_internal_add_resource(tst_qwebenginecookiestore "tst_qwebenginecookiestore"
    PREFIX
        "/"
    FILES
        ${tst_qwebenginecookiestore_resource_files}
)
