# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

qt_examples_build_begin(EXTERNAL_BUILD)

find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Pdf PdfWidgets)

if(TARGET Qt::WebEngineCore)
    add_subdirectory(webenginequick)
endif()
if(TARGET Qt::WebEngineWidgets)
    add_subdirectory(webenginewidgets)
endif()
if(TARGET Qt::Pdf)
    add_subdirectory(pdf)
endif()
if(TARGET Qt::PdfWidgets)
    add_subdirectory(pdfwidgets)
endif()
qt_examples_build_end()
