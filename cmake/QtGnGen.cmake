# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This is gn wrapper script and it assables final BUILD.gn based on:
# gn_config_target.cmake gn_config_c.cmake gn_config_cxx.cmake

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message("This files should run only in script mode")
    return()
endif()

set(path_mode REALPATH)
if(APPLE AND QT_ALLOW_SYMLINK_IN_PATHS)
    set(path_mode ABSOLUTE)
endif()

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ${path_mode})
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" ${path_mode})

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
if(NOT Python3_EXECUTABLE)
    find_package(Python3 3.6 REQUIRED)
endif()
set(gnCmd ${Gn_EXECUTABLE})
set(buildDir ${BUILD_DIR})
set(sourceDir ${SOURCE_DIR})
set(module ${MODULE})
set(gnArg gen ${buildDir})
file(READ ${buildDir}/args.gn gnArgArg)

if(NOT gnCmd)
    message(FATAL_ERROR "\nCould not find suitable gn to run.\n")
endif()

init_gn_config(${buildDir}/gn_config_target.cmake)
read_gn_config(${buildDir}/gn_config_cxx.cmake)
read_gn_config(${buildDir}/gn_config_c.cmake)
read_gn_config(${buildDir}/gn_static.cmake)

configure_gn_target(
   "${sourceDir}"
   "${WEBENGINE_ROOT_SOURCE_DIR}/src/${module}/configure/BUILD.root.gn.in"
   "${buildDir}/BUILD.gn"
)

list(APPEND gnArg
     --script-executable=${Python3_EXECUTABLE}
     --root=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium)

if(GN_THREADS)
   list(APPEND gnArg --threads=${GN_THREADS})
endif()

STRING(REGEX REPLACE "\n" ";" printArgArg "${gnArgArg}")
LIST(SORT printArgArg)
STRING(REGEX REPLACE ";" "\n" printArgArg "${printArgArg}")
list(JOIN gnArg " " printArg)

message("-- Running gn in ${buildDir}\n"
        "-- GN command:\n${gnCmd} ${printArg}\n"
        "-- GN arg file:\n${buildDir}/args.gn\n"
        "-- GN args: \n${printArgArg}"
)

execute_process(
    COMMAND ${gnCmd} ${gnArg}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE gnResult
    OUTPUT_VARIABLE gnOutput
    ERROR_VARIABLE gnError
    TIMEOUT 600
)

if(NOT gnResult EQUAL 0)
    message(FATAL_ERROR "\n-- GN FAILED\n${gnOutput}\n${gnError}\n${gnResult}\n")
else()
    string(REGEX REPLACE "\n$" "" gnOutput "${gnOutput}")
    message("-- GN ${gnOutput}")
endif()
