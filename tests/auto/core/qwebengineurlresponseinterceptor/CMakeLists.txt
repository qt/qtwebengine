# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_qwebengineurlresponseinterceptor
    SOURCES
        tst_qwebengineurlresponseinterceptor.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
)
