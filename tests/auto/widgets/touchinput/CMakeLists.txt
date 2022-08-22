# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_touchinput
    SOURCES
        tst_touchinput.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Qt::GuiPrivate
        Test::Util
)
