# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message("This files should run only in script mode")
    return()
endif()

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
if(NOT Python3_EXECUTABLE)
    find_package(Python3 3.6 REQUIRED)
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${LICENSE_SCRIPT}
        --file-template ${FILE_TEMPLATE}
        --entry-template ${ENTRY_TEMPLATE}
        --gn-binary ${Gn_EXECUTABLE}
        --gn-target ${GN_TARGET}
        --gn-out-dir ${BUILDDIR}
        credits ${OUTPUT}
    WORKING_DIRECTORY ${BUILDDIR}
    RESULT_VARIABLE gnResult
    OUTPUT_VARIABLE gnOutput
    ERROR_VARIABLE gnError
    TIMEOUT 600
)

if(NOT gnResult EQUAL 0)
    message(FATAL_ERROR "\n-- License FAILED\n${gnOutput}\n${gnError}\n${gnResult}\n")
else()
    string(REGEX REPLACE "\n$" "" gnOutput "${gnOutput}")
    message("-- License ${gnOutput}")
endif()
