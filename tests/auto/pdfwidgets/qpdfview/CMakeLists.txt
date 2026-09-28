# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfview
    SOURCES
        tst_qpdfview.cpp
    LIBRARIES
        Qt::Gui
        Qt::Widgets
        Qt::PdfPrivate
        Qt::PdfWidgets
    TESTDATA
        qpdfwriter.pdf
)
