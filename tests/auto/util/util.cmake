# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if (NOT TARGET Test::Util)
   add_library(qtestutil INTERFACE)
   target_include_directories(qtestutil INTERFACE ${CMAKE_CURRENT_LIST_DIR})
   add_library(Test::Util ALIAS qtestutil)
endif()
