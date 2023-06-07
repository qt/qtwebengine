# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_uidelegates
    SOURCES
        tst_uidelegates.cpp
    LIBRARIES
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuick
        Qt::GuiPrivate
        Qt::WebEngineQuickPrivate
        Test::HttpServer
        Test::Util
)
