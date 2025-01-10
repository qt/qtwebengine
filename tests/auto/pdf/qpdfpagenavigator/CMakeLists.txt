# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfpagenavigator.cpp
    SOURCES
        tst_qpdfpagenavigator.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::Pdf
        Qt::PdfWidgets
    TESTDATA
        pdf-sample.bookmarks_pages.pdf
)
