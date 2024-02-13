# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are functions aim to help during configure step checks and build setup

macro(qt_webengine_run_configure configure_file_path)
    # Run main configure that does not belong to any module
    qt_feature_module_begin(ONLY_EVALUATE_FEATURES)
    qt_webengine_configure_begin()
    # Enable printing of feature summary by forcing qt_configure_record_command
    # to work in spite of ONLY_EVALUATE_FEATURES.
    set(__QtFeature_only_evaluate_features OFF)
    include(${configure_file_path})
    qt_webengine_configure_end()
    qt_feature_module_end(ONLY_EVALUATE_FEATURES)
endmacro()

function(qt_webengine_configure_begin)
    set(configure_checks "" PARENT_SCOPE)
endfunction()

# Sets QT_CONFIGURE_CHECK_(module)_build and prints found issues
function(qt_webengine_configure_end)

    foreach(module_checked ${configure_checks})
        set(error_message "\n -- The following configure issues were found:")
        string(TOLOWER ${module_checked} module)
        if(NOT ${configure_checks_${module}_support})
            foreach(check ${configure_checks_${module}})
                if(NOT ${configure_checks_${module}_${check}})
                    string(APPEND error_message "\n * ${configure_checks_${module}_${check}_error}")
                endif()
            endforeach()
            message(STATUS "Configure checks for ${module} failed.${error_message}")
            set(QT_CONFIGURE_CHECK_${module}_build OFF CACHE BOOL "Build ${module_checked} Modules" FORCE)
            qt_configure_add_report_entry(
               TYPE WARNING
               MESSAGE "${module_checked} won't be built. ${error_message}"
            )
            qt_webengine_add_error_target(${module_checked} "Delete CMakeCache.txt and try to reconfigure.")
        else()
           set(QT_CONFIGURE_CHECK_${module}_build ON CACHE BOOL "Build ${module_checked} Modules" FORCE)
        endif()
    endforeach()

    # Cleanup
    foreach(module_checked ${configure_checks})
        string(TOLOWER ${module_checked} module)
            foreach(check ${configure_checks_${module}})
                unset(configure_checks_${module}_${check}_error PARENT_SCOPE)
                unset(configure_checks_${module}_${check} PARENT_SCOPE)
            endforeach()
         unset(configure_checks_${module} PARENT_SCOPE)
         unset(configure_checks_${module}_support PARENT_SCOPE)
    endforeach()
    unset(configure_checks PARENT_SCOPE)
endfunction()

function(qt_webengine_add_error_target module error_message)
    add_custom_target(${module}_error_message ALL
        ${CMAKE_COMMAND} -E cmake_echo_color --red "${module} will not be built: ${error_message}"
        COMMENT "${module} configure check"
        VERBATIM
    )
endfunction()

function(qt_webengine_normalize_check name out_var)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" name "${name}")
    set(${out_var} "${name}" PARENT_SCOPE)
endfunction()

# Sets QT_CONFIGURE_CHECK_(check) according to CONDITION
function(qt_webengine_configure_check check)

    qt_webengine_normalize_check("${check}" check)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "" "" "MODULES;MESSAGE;CONDITION"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    foreach(m ${arg_MODULES})
        string(TOLOWER ${m} module)
        if(NOT DEFINED configure_checks_${module})
            set(configure_checks_${module} "" PARENT_SCOPE)
            set(configure_checks_${module}_support ON PARENT_SCOPE)
            set(configure_checks ${m} ${configure_checks})
            set(configure_checks ${configure_checks} PARENT_SCOPE)
        endif()
        if(NOT DEFINED configure_checks_${module}_${check})
            set(configure_checks_${module}_${check}_error "" PARENT_SCOPE)
            if("x${arg_CONDITION}" STREQUAL "x")
                set(arg_CONDITION ON)
            endif()
            qt_evaluate_config_expression(result ${arg_CONDITION})
            set(configure_checks_${module}_${check} ${result} PARENT_SCOPE)
            set(QT_CONFIGURE_CHECK_${check} ${result} PARENT_SCOPE)
            if(NOT ${result})
                set(configure_checks_${module}_support OFF PARENT_SCOPE)
                set(configure_checks_${module}_${check}_error ${arg_MESSAGE} PARENT_SCOPE)
            endif()
            set(configure_checks_${module} ${check} ${configure_checks_${module}} PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Duplicated config check '${check}' found. Aborting !")
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
