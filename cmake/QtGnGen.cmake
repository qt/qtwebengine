# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This is gn wrapper script and it assables final BUILD.gn based on:
#  * BUILD.root.gn.in
#  * gn_config_target.cmake
#  * gn_config_c.cmake
#  * gn_config_cxx.cmake
#  * gn_static.cmake


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
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtBuildGnHelpers.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
if(NOT Python3_EXECUTABLE)
    message(FATAL_ERROR "\nPython3_EXECUTABLE not set.\n")
endif()

set(gn_command ${Gn_EXECUTABLE})
set(build_dir ${BUILD_DIR})
set(source_dir ${SOURCE_DIR})
set(module ${MODULE})

if(NOT gn_command)
    message(FATAL_ERROR "\nCould not find suitable gn to run.\n")
endif()

##
#    CREATE BUILD.gn
##

init_gn_config(${build_dir}/gn_config_target.cmake)
read_gn_config(${build_dir}/gn_config_cxx.cmake)
read_gn_config(${build_dir}/gn_config_c.cmake)
read_gn_config(${build_dir}/gn_static.cmake)

configure_gn_target(
   "${source_dir}"
   "${WEBENGINE_ROOT_SOURCE_DIR}/src/${module}/configure/BUILD.root.gn.in"
   "${build_dir}/BUILD.gn"
   ${path_mode}
)

##
#    RUN GN COMMAND
##

set(gn_arg gen ${build_dir})
list(APPEND gn_arg
     --script-executable=${Python3_EXECUTABLE}
     --root=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium)

if(GN_THREADS)
   list(APPEND gn_arg --threads=${GN_THREADS})
endif()

file(READ ${build_dir}/args.gn gn_arg_arg)
STRING(REGEX REPLACE "\n" ";" print_arg_arg "${gn_arg_arg}")
LIST(SORT print_arg_arg)
STRING(REGEX REPLACE ";" "\n" print_arg_arg "${print_arg_arg}")
list(JOIN gn_arg " " printArg)

message("-- Running gn in ${build_dir}\n"
        "-- GN command:\n${gn_command} ${printArg}\n"
        "-- GN arg file:\n${build_dir}/args.gn\n"
        "-- GN args: \n${print_arg_arg}"
)

execute_process(
    COMMAND ${gn_command} ${gn_arg}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE gn_result
    OUTPUT_VARIABLE gn_output
    ERROR_VARIABLE gn_error
    TIMEOUT 600
)

if(NOT gn_result EQUAL 0)
    message(FATAL_ERROR "\n-- GN FAILED\n${gn_output}\n${gn_error}\n${gn_result}\n")
else()
    string(REGEX REPLACE "\n$" "" gn_output "${gn_output}")
    message("-- GN ${gn_output}")
endif()
