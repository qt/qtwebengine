# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qtbug-70248
    SOURCES
        tst_qtbug-70248.cpp
    LIBRARIES
        Qt::WebEngineQuickPrivate
)

set(test_resource_files
    "test.qml"
)

qt_internal_add_resource(tst_qtbug-70248 "test"
    PREFIX
        "/"
    FILES
        ${test_resource_files}
)
