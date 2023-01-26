# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_qtbug_110287
    SOURCES
        tst_qtbug_110287.cpp
    LIBRARIES
        Qt::Network
        Qt::WebEngineWidgets
)
target_link_options(tst_qtbug_110287 PRIVATE "-Wl,--as-needed")
