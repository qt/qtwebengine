# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_qpdfdocument
    SOURCES
        tst_qpdfdocument.cpp
    LIBRARIES
        Qt::Gui
        Qt::Network
        Qt::PrintSupport
        Qt::Pdf
)
