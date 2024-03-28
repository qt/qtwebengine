# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qtbug_110287
    SOURCES
        tst_qtbug_110287.cpp
    LIBRARIES
        Qt::Network
        Qt::WebEngineWidgets
)
target_link_options(tst_qtbug_110287 PRIVATE "-Wl,--as-needed")
