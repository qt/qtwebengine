# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qquickwebenginedefaultsurfaceformat
    SOURCES
        tst_qquickwebenginedefaultsurfaceformat.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
        Test::Util
)
