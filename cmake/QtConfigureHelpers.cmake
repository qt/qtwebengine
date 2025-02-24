# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are functions aim to help during configure step checks and build setup

macro(qt_webengine_run_configure configure_file_path)
    # Run main configure that does not belong to any module
    qt_feature_module_begin(ONLY_EVALUATE_FEATURES)
    # Enable printing of feature summary by forcing qt_configure_record_command
    # to work in spite of ONLY_EVALUATE_FEATURES.
    set(__QtFeature_only_evaluate_features OFF)
    include(${configure_file_path})
    qt_webengine_version_sanity_check()
    qt_webengine_check_support()
    qt_feature_module_end(ONLY_EVALUATE_FEATURES)
endmacro()

function(qt_webengine_configure_begin)
    set(configure_checks "" PARENT_SCOPE)
    set(configure_versions "" PARENT_SCOPE)
endfunction()

# Sets QT_CONFIGURE_CHECK_(module)_build and prints found issues and does the cleanup
function(qt_webengine_configure_end)
    qt_webengine_cleanup_configure_checks()
    qt_webengine_cleanup_configure_versions()
endfunction()

macro(qt_webengine_version_sanity_check)
    foreach(check ${configure_versions})
        if(NOT DEFINED QT_CONFIGURE_CHECK_${check})
            message(WARNING "Version for check '${check}' defined but no configure check for it")
        endif()
    endforeach()
endmacro()

macro(qt_webengine_check_support)
    foreach(module_checked ${configure_checks})
        set(error_message "The following configure errors were found:")
        set(warning_message "The following configure warnings were found:")
        string(TOLOWER ${module_checked} module)
        set(QT_CONFIGURE_CHECK_${module}_build ON CACHE BOOL "Build ${module_checked} Modules" FORCE)
        if(${configure_checks_${module}_error} OR ${configure_checks_${module}_warning})
            foreach(check ${configure_checks_${module}})
                if(NOT ${configure_checks_${module}_${check}})
                    if(NOT ${configure_checks_${module}_${check}_optional})
                        string(APPEND error_message "\n * ${configure_checks_${module}_${check}_error}")
                    else()
                        string(APPEND warning_message "\n * ${configure_checks_${module}_${check}_error}")
                    endif()
                endif()
            endforeach()
            if(${configure_checks_${module}_warning})
                message(STATUS "Configure checks for ${module} found issues.${warning_message}")
                qt_configure_add_report_entry(
                    TYPE WARNING
                    MESSAGE "${module_checked} has warnings. ${warning_message}"
                )
                endif()
            if(${configure_checks_${module}_error})
                message(STATUS "Configure checks for ${module} failed.${error_message}")
                set(QT_CONFIGURE_CHECK_${module}_build OFF CACHE BOOL "Build ${module_checked} Modules" FORCE)
                qt_configure_add_report_entry(
                    TYPE WARNING
                    MESSAGE "${module_checked} won't be built. ${error_message}"
                )
                qt_webengine_add_error_target(${module_checked} "Delete CMakeCache.txt and try to reconfigure.")
            endif()
        endif()
    endforeach()
endmacro()

macro(qt_webengine_cleanup_configure_checks)
  foreach(module_checked ${configure_checks})
        string(TOLOWER ${module_checked} module)
            foreach(check ${configure_checks_${module}})
                unset(configure_checks_${module}_${check}_error PARENT_SCOPE)
                unset(configure_checks_${module}_${check} PARENT_SCOPE)
                unset(configure_checks_${module}_${check}_optional PARENT_SCOPE)
            endforeach()
         unset(configure_checks_${module} PARENT_SCOPE)
         unset(configure_checks_${module}_error PARENT_SCOPE)
         unset(configure_checks_${module}_warning PARENT_SCOPE)
         unset(configure_checks_${module}_documentation PARENT_SCOPE)
    endforeach()
    unset(configure_checks PARENT_SCOPE)
endmacro()

macro(qt_webengine_cleanup_configure_versions)
    foreach(check ${configure_versions})
       unset(QT_CONFIGURE_CHECK_${check}_version PARENT_SCOPE)
    endforeach()
    unset(configure_versions PARENT_SCOPE)
endmacro()

function(qt_webengine_generate_documentation in_file_path out_file_path)
    foreach(tag ${configure_checks_qtwebengine_documentation})
        foreach(doc ${configure_checks_qtwebengine_documentation_${tag}})
            string(APPEND DOCUMENTATION_${tag} "\n \\li ${doc}")
        endforeach()
    endforeach()
    configure_file(${in_file_path} ${out_file_path} @ONLY)
endfunction()

function(qt_webengine_set_version check version)
    if(NOT DEFINED QT_CONFIGURE_CHECK_${check}_version)
        set(QT_CONFIGURE_CHECK_${check}_version ${version} PARENT_SCOPE)
        set(configure_versions ${configure_versions} ${check} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Version '${check}' redefined. Aborting ...")
    endif()
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
        "OPTIONAL" "" "MODULES;MESSAGE;CONDITION;TAGS;DOCUMENTATION"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    foreach(m ${arg_MODULES})
        string(TOLOWER ${m} module)
        if(NOT DEFINED configure_checks_${module})
            set(configure_checks_${module} "" PARENT_SCOPE)
            set(configure_checks_${module}_error OFF PARENT_SCOPE)
            set(configure_checks_${module}_warning OFF PARENT_SCOPE)
            set(configure_checks_${module}_documentation "" PARENT_SCOPE)
            set(configure_checks ${m} ${configure_checks})
            set(configure_checks ${configure_checks} PARENT_SCOPE)
        endif()
        if(NOT DEFINED configure_checks_${module}_${check})
            set(configure_checks_${module}_${check}_error "" PARENT_SCOPE)
            if("x${arg_CONDITION}" STREQUAL "x")
                set(arg_CONDITION ON)
            endif()
            if(NOT "x${arg_PLATFORM}" STREQUAL "x")
                set(arg_CONDITION NOT ${arg_PLATFORM} OR ${arg_CONDITION})
            endif()
            qt_evaluate_config_expression(result ${arg_CONDITION})
            set(configure_checks_${module}_${check} ${result} PARENT_SCOPE)
            set(QT_CONFIGURE_CHECK_${check} ${result} PARENT_SCOPE)
            if(NOT ${result})
                if(NOT ${arg_OPTIONAL})
                    set(configure_checks_${module}_error ON PARENT_SCOPE)
                else()
                    set(configure_checks_${module}_warning ON PARENT_SCOPE)
                endif()
                set(configure_checks_${module}_${check}_error ${arg_MESSAGE} PARENT_SCOPE)
            endif()
            if(NOT ${arg_OPTIONAL})
                set(configure_checks_${module}_${check}_optional FALSE PARENT_SCOPE)
            else()
                set(configure_checks_${module}_${check}_optional TRUE PARENT_SCOPE)
            endif()
            if(DEFINED arg_DOCUMENTATION)
                if(NOT DEFINED arg_TAGS)
                    set(arg_TAGS ALL_PLATFORMS)
                endif()
                foreach(tag ${arg_TAGS})
                    if(NOT DEFINED configure_checks_${module}_documentation_${tag})
                        set(configure_checks_${module}_documentation
                            ${configure_checks_${module}_documentation} ${tag} PARENT_SCOPE)
                    endif()
                    set(configure_checks_${module}_documentation_${tag}
                        ${configure_checks_${module}_documentation_${tag}}
                        ${arg_DOCUMENTATION} PARENT_SCOPE
                    )
                endforeach()
            endif()
            set(configure_checks_${module} ${check} ${configure_checks_${module}} PARENT_SCOPE)
       else()
           message(FATAL_ERROR "Duplicated config check '${check}' found. Aborting !")
       endif()
    endforeach()
endfunction()

function(qt_webengine_configure_check_for_ulimit)
    message(STATUS "Checking 'ulimit -n'")
    execute_process(COMMAND bash -c "ulimit -n"
        OUTPUT_VARIABLE ulimit_output
    )
    string(REGEX MATCHALL "[0-9]+" limit "${ulimit_output}")
    message(STATUS "Open files limit ${limit}")
    if(NOT (QT_FEATURE_use_gold_linker OR QT_FEATURE_use_lld_linker) AND ulimit_output LESS 4096)
        if(NOT ${CMAKE_VERSION} VERSION_LESS "3.21.0")
            message(STATUS "Creating linker launcher")
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
    string(REGEX REPLACE "([0-9.]+).*" "\\1" windows_sdk_version "${windows_sdk_version}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" sdk_minor "${windows_sdk_version}")
    set(${result_sdk_full} "${windows_sdk_version}" PARENT_SCOPE)
    set(${result_sdk_minor} "${sdk_minor}" PARENT_SCOPE)
endfunction()

function(qt_webengine_configure_check_coin white_list_file webengine_check pdf_check)
    if("${webengine_check}" STREQUAL "" OR "${pdf_check}" STREQUAL "")
        return()
    endif()
    set(coin_id $ENV{COIN_PLATFORM_ID})
    if(coin_id)
        set(res OFF)
        file(READ ${white_list_file} white_list)
        string(REPLACE "\n" ";" white_list "${white_list}")
        foreach(line IN LISTS white_list)
            string(REPLACE " " ";" line "${line}")
            list(REMOVE_ITEM line "")
            if(NOT line)
                continue()
            endif()
            list(POP_FRONT line id)
            if(id MATCHES "^#") # skip comments
                continue()
            endif()
            if(id)
               set(${id} ${line})
            endif()
        endforeach()
        if(DEFINED ${coin_id})
            list(POP_FRONT ${coin_id} webengine_build)
            list(POP_FRONT ${coin_id} pdf_build)
            set(res ON)
            if(webengine_build AND NOT webengine_check)
                set(res OFF)
            endif()
            if(pdf_build AND NOT pdf_check)
                set(res OFF)
            endif()
        else()
            message(WARNING "Undefined coin sanity check for ${coin_id} platform")
            return()
        endif()
        if(NOT res)
            message(FATAL_ERROR "!!! Coin sanity check failed for ${coin_id} platform with:
           (qtwebegine=${QT_CONFIGURE_CHECK_qtwebengine_build} qtpdf=${QT_CONFIGURE_CHECK_qtpdf_build}), but expected: (qtwebengine=${webengine_build} qtpdf=${pdf_build})")
        endif()
    endif()
endfunction()
