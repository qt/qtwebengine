# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(APPLE AND CLANG AND NOT QT_NO_USE_WEBENGINE_LD_CLASSIC)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0.0" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0.0")
    target_link_options(Qt6::WebEngineCore PUBLIC -ld_classic)
  endif()
endif()
