# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This is gn wrapper script and it assables code attributions.


if(NOT CMAKE_SCRIPT_MODE_FILE)
    message("This files should run only in script mode")
    return()
endif()

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
if(NOT Python3_EXECUTABLE)
    message(FATAL_ERROR "\nPython3_EXECUTABLE not set.\n")
endif()

set(extra_third_party_dirs "")
if(NOT "${EXTRA_THIRD_PARTY_DIRS}" STREQUAL "")
    string(REPLACE " " ";" dir_list ${EXTRA_THIRD_PARTY_DIRS})
    foreach(dir ${dir_list})
        string(CONCAT extra_third_party_dirs ${extra_third_party_dirs}"${dir}",)
    endforeach()
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${LICENSE_SCRIPT}
        --file-template ${FILE_TEMPLATE}
        --entry-template ${ENTRY_TEMPLATE}
        --gn-binary ${Gn_EXECUTABLE}
        --gn-target ${GN_TARGET}
        --gn-out-dir ${BUILDDIR}
        --extra-third-party-dirs=[${extra_third_party_dirs}]
        credits ${OUTPUT}
    WORKING_DIRECTORY ${BUILDDIR}
    RESULT_VARIABLE gn_result
    OUTPUT_VARIABLE gn_output
    ERROR_VARIABLE gn_error
    TIMEOUT 600
)

if(NOT gn_result EQUAL 0)
    message(FATAL_ERROR "\n-- License FAILED\n${gn_output}\n${gn_error}\n${gn_result}\n")
else()
    string(REGEX REPLACE "\n$" "" gn_output "${gn_output}")
    message("-- License ${gn_output}")
endif()
