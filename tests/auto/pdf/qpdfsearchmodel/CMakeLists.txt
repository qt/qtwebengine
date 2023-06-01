# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qpdfsearchmodel
    SOURCES
        tst_qpdfsearchmodel.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::Pdf
    TESTDATA
        test.pdf
)
