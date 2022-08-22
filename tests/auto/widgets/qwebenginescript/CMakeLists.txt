# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginescript
    SOURCES
        tst_qwebenginescript.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_qwebenginescript_resource_files
    "resources/test_iframe_inner.html"
    "resources/test_iframe_main.html"
    "resources/test_iframe_outer.html"
    "resources/test_window_open.html"
    "resources/title_a.html"
    "resources/title_b.html"
    "resources/webChannelWithBadString.html"
)

qt_internal_add_resource(tst_qwebenginescript "tst_qwebenginescript"
    PREFIX
        "/"
    FILES
        ${tst_qwebenginescript_resource_files}
)
