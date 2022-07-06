# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_plugin(QPdfPlugin
    OUTPUT_NAME qpdf
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qpdfiohandler.cpp qpdfiohandler_p.h
    LIBRARIES
        Qt::PdfPrivate
)

