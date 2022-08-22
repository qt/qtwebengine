# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qquickwebengineview
    SOURCES
        tst_qquickwebengineview.cpp
    LIBRARIES
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuick
        Qt::GuiPrivate
        Qt::WebEngineQuickPrivate
        Qt::TestPrivate
        Test::Util
)
