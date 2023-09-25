# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qpdfbookmarkmodel)
if (TARGET Qt::PdfWidgets)
    add_subdirectory(qpdfpagenavigator)
endif()
add_subdirectory(qpdfpagerenderer)
if(TARGET Qt::PrintSupport)
    add_subdirectory(qpdfdocument)
endif()
