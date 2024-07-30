# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_printing
    SOURCES
        tst_printing.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineWidgets
        Test::Util
)

qt_internal_extend_target(tst_printing
    CONDITION TARGET Qt::Pdf
    LIBRARIES
        Qt::Pdf
    DEFINES
        QTPDF_SUPPORT
)

set(tst_printing_resource_files
    "resources/basic_printing_page.html"
)

qt_internal_add_resource(tst_printing "tst_printing"
    PREFIX "/"
    FILES ${tst_printing_resource_files}
)
