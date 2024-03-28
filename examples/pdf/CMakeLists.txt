# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(singlepage)
qt_internal_add_example(multipage)
if(NOT TARGET Qt::Svg)
    message(WARNING "QtSvg is required as runtime dependency for qtpdfquick examples.")
endif()
