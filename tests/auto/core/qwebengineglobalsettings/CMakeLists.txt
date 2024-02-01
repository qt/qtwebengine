# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineglobalsettings
    SOURCES
        tst_qwebengineglobalsettings.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Qt::WebEngineWidgets
        Test::Util
)

# Resources:
set(tst_qwebengineglobalsettings_resource_files
    "cert/localhost.crt"
    "cert/localhost.key"
    "cert/RootCA.pem"
)

qt_add_resources(tst_qwebengineglobalsettings "tst_qwebengineglobalsettings"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineglobalsettings_resource_files}
)
