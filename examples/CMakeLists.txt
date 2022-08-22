# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

qt_examples_build_begin(EXTERNAL_BUILD)
if(NOT CMAKE_CROSSCOMPILING) #QTBUG-86533
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
endif()
qt_examples_build_end()
