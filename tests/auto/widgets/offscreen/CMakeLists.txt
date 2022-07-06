# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_offscreen
    SOURCES
        tst_offscreen.cpp
    LIBRARIES
        Qt::WebEngineWidgets
)

set(tst_offscreen_resource_files
    "test.html"
)

set_tests_properties(tst_offscreen PROPERTIES
    ENVIRONMENT QT_QPA_PLATFORM=offscreen
)

qt_internal_add_resource(tst_offscreen "tst_offscreen"
    PREFIX
        "/"
    FILES
        ${tst_offscreen_resource_files}
)

