# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qpdfbookmarkmodel)
#add_subdirectory(qpdfpagenavigator)
add_subdirectory(qpdfpagerenderer)
if(TARGET Qt::PrintSupport)
    add_subdirectory(qpdfdocument)
endif()
