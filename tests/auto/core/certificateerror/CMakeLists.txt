# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_certificateerror
    SOURCES
        tst_certificateerror.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Test::Util
)

set(tst_certificateerror_resource_files
    "resources/server.pem"
    "resources/server.key"
)

qt_internal_add_resource(tst_certificateerror "tst_certificateerror"
    PREFIX
       "/"
    FILES
       ${tst_certificateerror_resource_files}
)
