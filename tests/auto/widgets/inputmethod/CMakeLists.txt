# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_inputmethod
    SOURCES
        tst_inputmethod.cpp
    LIBRARIES
        Qt::GuiPrivate
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_inputmethod_resource_files
    "resources/input_modes.html"
    "resources/input_types.html"
    "resources/style.css"
)

qt_internal_add_resource(tst_inputmethod "tst_inputmethod"
    PREFIX
        "/"
    FILES
        ${tst_inputmethod_resource_files}
)
