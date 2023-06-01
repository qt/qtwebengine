# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_loadsignals
    SOURCES
        tst_loadsignals.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::HttpServer
        Test::Util
)

get_target_property(sharedData Test::HttpServer SHARED_DATA)

set_source_files_properties("${sharedData}/loadprogress/downloadable.tar.gz"
    PROPERTIES QT_RESOURCE_ALIAS "downloadable.tar.gz"
)
set_source_files_properties("${sharedData}/loadprogress/page1.html"
    PROPERTIES QT_RESOURCE_ALIAS "page1.html"
)
set_source_files_properties("${sharedData}/loadprogress/page2.html"
    PROPERTIES QT_RESOURCE_ALIAS "page2.html"
)
set_source_files_properties("${sharedData}/loadprogress/page3.html"
    PROPERTIES QT_RESOURCE_ALIAS "page3.html"
)
set_source_files_properties("${sharedData}/loadprogress/page4.html"
    PROPERTIES QT_RESOURCE_ALIAS "page4.html"
)
set_source_files_properties("${sharedData}/loadprogress/page5.html"
    PROPERTIES QT_RESOURCE_ALIAS "page5.html"
)
set_source_files_properties("${sharedData}/loadprogress/page6.html"
    PROPERTIES QT_RESOURCE_ALIAS "page6.html"
)
set_source_files_properties("${sharedData}/loadprogress/page7.html"
    PROPERTIES QT_RESOURCE_ALIAS "page7.html"
)
set_source_files_properties("${sharedData}/loadprogress/page8.html"
    PROPERTIES QT_RESOURCE_ALIAS "page8.html"
)

set(tst_loadsignals_resource_files
    "${sharedData}/loadprogress/downloadable.tar.gz"
    "${sharedData}/loadprogress/page1.html"
    "${sharedData}/loadprogress/page2.html"
    "${sharedData}/loadprogress/page3.html"
    "${sharedData}/loadprogress/page4.html"
    "${sharedData}/loadprogress/page5.html"
    "${sharedData}/loadprogress/page6.html"
    "${sharedData}/loadprogress/page7.html"
    "${sharedData}/loadprogress/page8.html"
)

qt_internal_add_resource(tst_loadsignals "tst_loadsignals"
    PREFIX
        "/resources"
    FILES
        ${tst_loadsignals_resource_files}
)
