# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

qt_internal_add_module(PdfWidgets
    SOURCES
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
