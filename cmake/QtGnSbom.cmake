# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This is a wrapper around sbom.py which creates an SBOM document for a GN-based target.

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This file should run only in script mode")
    return()
endif()

foreach(variable
        Python3_EXECUTABLE SCRIPT_PATH GN_TARGET_LIST BUILD_DIR_LIST
        PACKAGE_ID DOC_NAMESPACE OUTPUT)
    if(NOT DEFINED ${variable} OR "${${variable}}" STREQUAL "")
        message(FATAL_ERROR "\n${variable} not set (${${variable}})\n")
    endif()
endforeach()

set(path_mode REALPATH)
if(APPLE AND QT_ALLOW_SYMLINK_IN_PATHS)
    set(path_mode ABSOLUTE)
endif()

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ${path_mode})
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" ${path_mode})

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtBuildGnHelpers.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT MODULE REQUIRED)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${SCRIPT_PATH}"
        --gn-binary "${Gn_EXECUTABLE}"
        --gn-target-list "${GN_TARGET_LIST}"
        --build-dir-list "${BUILD_DIR_LIST}"
        --gn-version ${Gn_VERSION}
        --package-id ${PACKAGE_ID}
        --namespace "${DOC_NAMESPACE}"
        "${OUTPUT}"
    RESULT_VARIABLE gn_result
    OUTPUT_VARIABLE gn_output
    ERROR_VARIABLE gn_error
    TIMEOUT 600
)

if(NOT gn_result EQUAL 0)
    string(REGEX REPLACE "\n$" "" gn_output "${gn_output}")
    string(REGEX REPLACE "\n$" "" gn_error "${gn_error}")
    message(FATAL_ERROR "\n-- SBOM generation FAILED\n${gn_output}\n${gn_error}\n-- Exit code: ${gn_result}\n")
endif()
