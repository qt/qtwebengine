# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_example(pdfviewer)
qt_internal_add_example(multipage)
if(NOT TARGET Qt::Svg)
    message(WARNING "QtSvg is required as runtime dependency for qtpdfquick examples.")
endif()
