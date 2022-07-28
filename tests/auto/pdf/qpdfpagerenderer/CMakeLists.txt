# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_qpdfpagerenderer
    SOURCES
        tst_qpdfpagerenderer.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::Pdf
)

