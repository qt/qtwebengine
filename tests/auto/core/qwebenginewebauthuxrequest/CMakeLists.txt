# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginewebauthuxrequest
    SOURCES
        tst_qwebenginewebauthuxrequest.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::Util
)
