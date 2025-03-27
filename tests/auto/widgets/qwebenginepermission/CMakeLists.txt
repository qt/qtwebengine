# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginepermission
    SOURCES
        tst_qwebenginepermission.cpp
    LIBRARIES
        Qt::WebEngineCore
        Qt::WebEngineWidgets
        Qt::WebEngineCorePrivate
        Qt::CorePrivate
        Test::Util
)

set(tst_qwebenginepermission_resource_files
    "resources/index.html"
    "resources/iframe.html"
    "resources/qt144.png"
)

qt_internal_add_resource(tst_qwebenginepermission "tst_qwebenginepermission"
    PREFIX
        "/"
    FILES
        ${tst_qwebenginepermission_resource_files}
)
