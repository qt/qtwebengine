# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QPdfPlugin
    OUTPUT_NAME qpdf
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qpdfiohandler.cpp qpdfiohandler_p.h
    LIBRARIES
        Qt::PdfPrivate
)

