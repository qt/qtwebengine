# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfbookmarkmodel
    SOURCES
        tst_qpdfbookmarkmodel.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::Pdf
    TESTDATA
        pdf-sample.bookmarks.pdf
        pdf-sample.bookmarks_pages.pdf
)

