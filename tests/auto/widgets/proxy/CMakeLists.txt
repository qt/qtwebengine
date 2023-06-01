# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_webengine_proxy
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::WebEngineWidgets
       Test::HttpServer
)
