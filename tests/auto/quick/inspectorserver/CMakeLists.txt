# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_inspectorserver
    SOURCES
        tst_inspectorserver.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
)


