# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


##
#  WEBENGINECORE DICT CONVERT TOOL
##

if(QT_FEATURE_webengine_spellchecker AND NOT CMAKE_CROSSCOMPILING)
    qt_get_tool_target_name(dict_target_name qwebengine_convert_dict)
    qt_internal_add_tool(${dict_target_name}
        TARGET_DESCRIPTION "QtWebEngine Dictionary Conversion Tool"
        INSTALL_DIR ${INSTALL_LIBEXECDIR}
        TOOLS_TARGET WebEngineCore
        SOURCES main.cpp
        INCLUDE_DIRECTORIES
            ../../3rdparty/chromium/third_party/abseil-cpp
    )
    if(COMMAND qt_internal_return_unless_building_tools)
        qt_internal_return_unless_building_tools()
    endif()
    qt_skip_warnings_are_errors(${dict_target_name})
    add_dependencies(${dict_target_name} WebEngineCore)
    qt_internal_extend_target(${dict_target_name} CONDITION WIN32
        DEFINES WIN32_LEAN_AND_MEAN
    )
    qt_internal_extend_target(${dict_target_name} CONDITION GCC OR CLANG
        COMPILE_OPTIONS -Wno-unused-parameter
    )
    if(NOT QT_FEATURE_webengine_system_icu AND QT_WILL_INSTALL)
        # tool can be called durig build so copy icu file
        get_target_property(icuFile WebEngineCore ICUDTL_FILE)
        addCopyCommand(${dict_target_name} "${icuFile}"
            "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}")
    endif()
    qt_internal_extend_target(${dict_target_name}
        CONDITION NOT QT_FEATURE_webengine_system_icu
        DEFINES USE_ICU_FILE
    )
endif()

