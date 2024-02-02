# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

include(../../util/util.cmake)

qt_internal_add_test(tst_getdomainandregistry
    SOURCES
        tst_getdomainandregistry.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::Util
)
