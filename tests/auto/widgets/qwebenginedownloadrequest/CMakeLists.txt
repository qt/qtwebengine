# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
