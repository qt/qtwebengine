# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineview
    SOURCES
        tst_qwebengineview.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Qt::WebEngineCorePrivate
        Qt::GuiPrivate
        Qt::QuickWidgets
        Qt::TestPrivate
        Test::Util
)

set(tst_qwebengineview_resource_files
    "resources/dummy.html"
    "resources/frame_a.html"
    "resources/image2.png"
    "resources/index.html"
    "resources/input_types.html"
    "resources/keyboardEvents.html"
    "resources/scrolltest_page.html"
)

qt_internal_add_resource(tst_qwebengineview "tst_qwebengineview"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineview_resource_files}
)
