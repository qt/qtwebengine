# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# NOTE: This code should only ever be executed in script mode. It expects to be
#       used either as part of an install(CODE) call or called by a script
#       invoked via cmake -P as a POST_BUILD step. It would not normally be
#       included directly, it should be pulled in automatically by the deploy
#       support set up by qtbase.

cmake_minimum_required(VERSION 3.16...3.21)

_qt_internal_add_deployment_hook(_qt_internal_webenginecore_deploy_hook)

if(NOT QT_DEPLOY_WEBENGINECORE_RESOURCES_DIR)
    set(QT_DEPLOY_WEBENGINECORE_RESOURCES_DIR "resources")
endif()

function(_qt_internal_webenginecore_status_message)
    if(__QT_DEPLOY_VERBOSE)
        message(STATUS ${ARGV})
    endif()
endfunction()

function(_qt_internal_webenginecore_deploy_hook)
    set(no_value_options "")
    set(single_value_options "")
    set(multi_value_options RESOLVED_DEPENDENCIES)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${no_value_options}" "${single_value_options}" "${multi_value_options}"
    )

    set(webenginecore_dependency_found FALSE)
    foreach(dependency IN LISTS arg_RESOLVED_DEPENDENCIES)
        if(dependency MATCHES "/libQt[0-9]+WebEngineCore[^/]+")
            set(webenginecore_dependency_found TRUE)
            break()
        endif()
    endforeach()

    if(NOT webenginecore_dependency_found)
        _qt_internal_webenginecore_status_message(
            "No QtWebEngineCore dependency found. "
            "Skipping deployment of QtWebEngine assets."
        )
        return()
    endif()

    _qt_internal_deploy_webenginecore()
endfunction()

function(_qt_internal_deploy_webenginecore)
    _qt_internal_deploy_webenginecore_binary()
    _qt_internal_deploy_webenginecore_data()
    _qt_internal_deploy_webenginecore_translations()
endfunction()

function(_qt_internal_deploy_webenginecore_binary)
    _qt_internal_webenginecore_status_message("Deploying the WebEngineCore process binary")

    set(candidates "QtWebEngineProcess")
    if(__QT_DEPLOY_ACTIVE_CONFIG STREQUAL "Debug" AND __QT_DEPLOY_SYSTEM_NAME STREQUAL "Windows")
        list(PREPEND candidates "QtWebEngineProcessd")
    endif()

    list(TRANSFORM candidates
        PREPEND "${__QT_DEPLOY_QT_INSTALL_PREFIX}/${__QT_DEPLOY_QT_INSTALL_LIBEXECS}/"
    )

    set(process_path "")
    foreach(file_path IN LISTS candidates)
        if(EXISTS "${file_path}")
            set(process_path "${file_path}")
            break()
        endif()
    endforeach()

    set(install_destination "${QT_DEPLOY_PREFIX}/")
    if(__QT_DEPLOY_SYSTEM_NAME STREQUAL "Windows")
        string(APPEND install_destination "${QT_DEPLOY_BIN_DIR}")
    else()
        if(NOT DEFINED QT_DEPLOY_LIBEXEC_DIR)
            set(QT_DEPLOY_LIBEXEC_DIR "libexec")
        endif()
        string(APPEND install_destination "${QT_DEPLOY_LIBEXEC_DIR}")
    endif()
    file(INSTALL "${process_path}" DESTINATION "${install_destination}")

    get_filename_component(process_file_name "${process_path}" NAME)
    if(CMAKE_VERSION GREATER_EQUAL "3.19")
        file(CHMOD "${install_destination}/${process_file_name}"
            PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
                        GROUP_EXECUTE GROUP_READ
                        WORLD_EXECUTE WORLD_READ
        )
    else()
        execute_process(
            COMMAND chmod 0755 "${install_destination}/${process_file_name}"
        )
    endif()
endfunction()

function(_qt_internal_deploy_webenginecore_data)
    _qt_internal_webenginecore_status_message("Deploying the WebEngineCore data files")
    set(data_files
        icudtl.dat
        qtwebengine_devtools_resources.pak
        qtwebengine_resources.pak
        qtwebengine_resources_100p.pak
        qtwebengine_resources_200p.pak
    )
    get_filename_component(resources_dir "resources" ABSOLUTE
        BASE_DIR "${__QT_DEPLOY_QT_INSTALL_PREFIX}/${__QT_DEPLOY_QT_INSTALL_DATA}"
    )

    _qt_internal_webenginecore_find_v8_context_snapshot(
        snapshot_file
        RESOURCES_DIR "${resources_dir}"
    )
    if(NOT snapshot_file STREQUAL "")
        list(APPEND data_files "${snapshot_file}")
    endif()

    get_filename_component(install_destination "${QT_DEPLOY_WEBENGINECORE_RESOURCES_DIR}" ABSOLUTE
        BASE_DIR "${QT_DEPLOY_PREFIX}/${QT_DEPLOY_DATA_DIR}"
    )
    foreach(data_file IN LISTS data_files)
        file(INSTALL "${resources_dir}/${data_file}" DESTINATION "${install_destination}")
    endforeach()
endfunction()

# The V8 snapshot file comes as debug or release build. Multi-config builds have both, a self-built
# Qt might only have the debug one.
#
# This function returns the file name of the V8 context snapshot file in ${out_var}.
# If no snapshot could be found, ${out_var} is the empty string.
function(_qt_internal_webenginecore_find_v8_context_snapshot out_var)
     set(no_value_options "")
     set(single_value_options RESOURCES_DIR)
     set(multi_value_options "")
     cmake_parse_arguments(PARSE_ARGV 1 arg
         "${no_value_options}" "${single_value_options}" "${multi_value_options}"
     )

    set(result "")
    set(candidates
        v8_context_snapshot.bin
        v8_context_snapshot.debug.bin
    )
    if(__QT_DEPLOY_QT_IS_MULTI_CONFIG_BUILD_WITH_DEBUG
            AND __QT_DEPLOY_ACTIVE_CONFIG STREQUAL "Debug")
        # Favor the debug version of the snapshot.
        list(REVERSE candidates)
    endif()
    foreach(candidate IN LISTS candidates)
        if(EXISTS "${arg_RESOURCES_DIR}/${candidate}")
            set(result "${candidate}")
            break()
        endif()
    endforeach()
    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()

function(_qt_internal_deploy_webenginecore_translations)
    _qt_internal_webenginecore_status_message("Deploying the WebEngineCore translations")

    get_filename_component(locales_dir "qtwebengine_locales" ABSOLUTE
        BASE_DIR "${__QT_DEPLOY_QT_INSTALL_PREFIX}/${__QT_DEPLOY_QT_INSTALL_TRANSLATIONS}"
    )
    get_filename_component(install_destination "qtwebengine_locales" ABSOLUTE
        BASE_DIR "${QT_DEPLOY_PREFIX}/${QT_DEPLOY_TRANSLATIONS_DIR}"
    )
    file(GLOB locale_files "${locales_dir}/*.pak")
    foreach(locale_file IN LISTS locale_files)
        file(INSTALL "${locale_file}" DESTINATION "${install_destination}")
    endforeach()
endfunction()
