# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginehistory
    SOURCES
        tst_qwebenginehistory.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_qwebenginehistory_resource_files
    "resources/page1.html"
    "resources/page2.html"
    "resources/page3.html"
    "resources/page4.html"
    "resources/page5.html"
    "resources/page6.html"
)

qt_internal_add_resource(tst_qwebenginehistory "tst_qwebenginehistory"
    PREFIX
        "/"
    FILES
        ${tst_qwebenginehistory_resource_files}
)

