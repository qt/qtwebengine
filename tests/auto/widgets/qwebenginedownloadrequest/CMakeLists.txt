# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginedownloadrequest
    SOURCES
        tst_qwebenginedownloadrequest.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::HttpServer
        Test::Util
)
