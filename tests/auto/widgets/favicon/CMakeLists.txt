# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

include(../../util/util.cmake)

qt_internal_add_test(tst_favicon
    SOURCES
        tst_favicon.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_favicon_resource_files
    "resources/favicon-misc.html"
    "resources/favicon-multi.html"
    "resources/favicon-shortcut.html"
    "resources/favicon-single.html"
    "resources/favicon-touch.html"
    "resources/favicon-unavailable.html"
    "resources/icons/qt144.png"
    "resources/icons/qt32.ico"
    "resources/icons/qtmulti.ico"
    "resources/test1.html"
)

qt_internal_add_resource(tst_favicon "tst_favicon"
    PREFIX
        "/"
    FILES
        ${tst_favicon_resource_files}
)
