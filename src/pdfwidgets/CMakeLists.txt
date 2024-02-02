# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

qt_internal_add_module(PdfWidgets
    SOURCES
        qpdfpageselector.cpp qpdfpageselector.h qpdfpageselector_p.h
        qpdfview.cpp qpdfview.h qpdfview_p.h
        qtpdfwidgetsglobal.h
    LIBRARIES
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Pdf
)
