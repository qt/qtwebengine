# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfpagerenderer
    SOURCES
        tst_qpdfpagerenderer.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::Pdf
    TESTDATA
        pdf-sample.pagerenderer.pdf
)

