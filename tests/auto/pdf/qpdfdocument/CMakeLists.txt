# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfdocument
    SOURCES
        tst_qpdfdocument.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::PrintSupport
        Qt::Pdf
    TESTDATA
        pdf-sample.protected.pdf
        pdf-sample.metadata.pdf
        test.pdf
)
