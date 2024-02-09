# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are functions aim to help during configure step checks and build setup

function(qt_webengine_configure_check)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "" "" "MODULES;MESSAGE;CONDITION"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    foreach(module ${arg_MODULES})
        if(NOT DEFINED ${module}_SUPPORT)
            set(${module}_SUPPORT ON PARENT_SCOPE)
            set(${module}_SUPPORT ON)
        endif()
        if(${module}_SUPPORT)
            if("x${arg_CONDITION}" STREQUAL "x")
                set(arg_CONDITION ON)
            endif()
            qt_evaluate_config_expression(result ${arg_CONDITION})
            if(NOT ${result})
                set(${module}_SUPPORT OFF PARENT_SCOPE)
                set(${module}_ERROR ${arg_MESSAGE} PARENT_SCOPE)
            qt_configure_add_report_entry(TYPE WARNING
                MESSAGE "${module} won't be built. ${arg_MESSAGE}"
                CONDITION ON
            )
            endif()
        endif()
    endforeach()
endfunction()

function(qt_webengine_configure_check_for_ulimit)
    message("-- Checking 'ulimit -n'")
    execute_process(COMMAND bash -c "ulimit -n"
        OUTPUT_VARIABLE ulimit_output
    )
    string(REGEX MATCHALL "[0-9]+" limit "${ulimit_output}")
    message(" -- Open files limit ${limit}")
    if(NOT (QT_FEATURE_use_gold_linker OR QT_FEATURE_use_lld_linker) AND ulimit_output LESS 4096)
        if(NOT ${CMAKE_VERSION} VERSION_LESS "3.21.0")
            message(" -- Creating linker launcher")
            file(GENERATE OUTPUT ${PROJECT_BINARY_DIR}/linker_ulimit.sh
                CONTENT "#!/bin/bash\nulimit -n 4096\nexec \"$@\""
                FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            )
            set(COIN_BUG_699 ON PARENT_SCOPE)
        else()
            set(PRINT_BFD_LINKER_WARNING ON PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Configures multi/matrix build

function(qt_webengine_add_build feature value)
    list(APPEND cmake_args
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DMATRIX_SUBBUILD=ON"
        "-DFEATURE_${feature}=${value}"
    )
    if(CMAKE_C_COMPILER_LAUNCHER)
        list(APPEND cmake_args "-DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}")
    endif()
    if(CMAKE_CXX_COMPILER_LAUNCHER)
        list(APPEND cmake_args "-DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}")
    endif()

    externalproject_add(${feature}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${feature}-${value}
        PREFIX ${feature}-${value}
        CMAKE_ARGS ${cmake_args}
        USES_TERMINAL_BUILD ON
        USES_TERMINAL_CONFIGURE ON
        BUILD_ALWAYS TRUE
        INSTALL_COMMAND ""
    )
    get_property(dep_tracker GLOBAL PROPERTY MATRIX_DEPENDENCY_TRACKER)
    foreach(dep ${dep_tracker})
        add_dependencies(${feature} ${dep})
    endforeach()
    set(dep_tracker "${dep_tracker}" ${feature})
    set_property(GLOBAL PROPERTY MATRIX_DEPENDENCY_TRACKER "${dep_tracker}")
endfunction()

function(qt_webengine_is_file_inside_root_build_dir out_var file)
    set(result ON)
    if(NOT QT_CONFIGURE_RUNNING)
        file(RELATIVE_PATH relpath "${WEBENGINE_ROOT_BUILD_DIR}" "${file}")
        if(IS_ABSOLUTE "${relpath}" OR relpath MATCHES "^\\.\\./")
            set(result OFF)
        endif()
    endif()
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

function(qt_webengine_get_windows_sdk_version result_sdk_full result_sdk_minor)
    set(windows_sdk_version $ENV{WindowsSDKVersion})
    String(REGEX REPLACE "([0-9.]+).*" "\\1" windows_sdk_version "${windows_sdk_version}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" sdk_minor "${windows_sdk_version}")
    set(${result_sdk_full} "${windows_sdk_version}" PARENT_SCOPE)
    set(${result_sdk_minor} "${sdk_minor}" PARENT_SCOPE)
endfunction()
