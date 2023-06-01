# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_dialogs
    SOURCES
        testhandler.cpp testhandler.h
        tst_dialogs.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
        Test::HttpServer
        Test::Util
)

set(dialogs_resource_files
    "WebView.qml"
    "index.html"
)

qt_internal_add_resource(tst_dialogs "dialogs"
    PREFIX
        "/"
    FILES
        ${dialogs_resource_files}
)
