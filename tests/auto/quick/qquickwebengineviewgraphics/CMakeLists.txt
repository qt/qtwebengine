# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)
qt_internal_add_test(tst_qquickwebengineviewgraphics
    SOURCES
        tst_qquickwebengineviewgraphics.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
        Qt::Test
        Test::Util
)
