# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginepage
    SOURCES
        tst_qwebenginepage.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::NetworkPrivate
        Qt::WebEngineCorePrivate
        Qt::WebEngineWidgets
        Test::HttpServer
        Test::Util
)

get_target_property(sharedData Test::HttpServer SHARED_DATA)

set(tst_qwebenginepage_resource_files
    "resources/redirect.html"
    "resources/bar.txt"
    "resources/content.html"
    "resources/dynamicFrame.html"
    "resources/foo.txt"
    "resources/frame_a.html"
    "resources/frame_c.html"
    "resources/framedindex.html"
    "resources/fullscreen.html"
    "resources/iframe.html"
    "resources/iframe2.html"
    "resources/iframe3.html"
    "resources/image.png"
    "resources/index.html"
    "resources/lifecycle.html"
    "resources/pasteimage.html"
    "resources/path with spaces.txt"
    "resources/reload.html"
    "resources/script.html"
    "resources/style.css"
    "resources/test1.html"
    "resources/test2.html"
    "resources/testiframe.html"
    "resources/testiframe2.html"
    "resources/user.css"
)

qt_internal_add_resource(tst_qwebenginepage "tst_qwebenginepage"
    PREFIX
        "/"
    FILES
        ${tst_qwebenginepage_resource_files}
)
set_source_files_properties("${sharedData}/notification.html"
    PROPERTIES QT_RESOURCE_ALIAS "notification.html"
)
set(tst_qwebenginepage1_resource_files
    "${sharedData}/notification.html"
)

qt_internal_add_resource(tst_qwebenginepage "tst_qwebenginepage1"
    PREFIX
        "/shared"
    FILES
        ${tst_qwebenginepage1_resource_files}
)
