# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT TARGET Test::Util)
   add_library(qtestutil INTERFACE)
   target_include_directories(qtestutil INTERFACE ${CMAKE_CURRENT_LIST_DIR})
   add_library(Test::Util ALIAS qtestutil)
endif()
