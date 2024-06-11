# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_schemes
    SOURCES
        tst_schemes.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

