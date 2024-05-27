# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineframe
    SOURCES
        tst_qwebengineframe.cpp
    LIBRARIES
        Qt::WebEngineCore
        Qt::WebEngineWidgets
        Test::Util
)

qt_internal_add_resource(tst_qwebengineframe "tst_qwebengineframe"
    PREFIX
        "/"
    FILES
        "resources/frameset.html"
        "resources/iframes.html"
        "resources/nesting-iframe.html"
        "resources/printing-inner-document.html"
        "resources/printing-outer-document.html"
)
