# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

add_subdirectory(qpdfbookmarkmodel)
#add_subdirectory(qpdfpagenavigator)
add_subdirectory(qpdfpagerenderer)
if(TARGET Qt::PrintSupport)
    add_subdirectory(qpdfdocument)
endif()
