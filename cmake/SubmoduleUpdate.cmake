# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message("This file should run only in script mode")
    return()
endif()

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

find_program(GIT_EXECUTABLE NAMES git REQUIRED)

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty
    OUTPUT_VARIABLE SUBMODULE_NEW_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD:src/3rdparty
    WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}
    OUTPUT_VARIABLE SUBMODULE_OLD_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(shas "${SUBMODULE_OLD_SHA}..${SUBMODULE_NEW_SHA}")
set(format "* %s")
execute_process(
    COMMAND ${GIT_EXECUTABLE} log
        --pretty=format:${format}
        --abbrev-commit ${shas}
    WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty
    OUTPUT_VARIABLE SUBMODULE_COMMITS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(SUBMODULE_COMMITS)
   message("commits found for ${shas}")
   execute_process(
       COMMAND ${GIT_EXECUTABLE} add src/3rdparty
       WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}
   )
   set(commits ${SUBMODULE_COMMITS})
   execute_process(
       COMMAND ${GIT_EXECUTABLE} commit
           --allow-empty
           -m "Update Chromium"
           -m "Submodule src/3rdparty ${shas}:\n${commits}"
       WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}
       RESULT_VARIABLE gitResult
       OUTPUT_VARIABLE gitOutput
       ERROR_VARIABLE gitError
   )

   if(NOT gitResult EQUAL 0)
       message(FATAL_ERROR "\n-- Git Commit FAILED\n${gitOutput}\n${gitError}\n${gitResult}\n")
   else()
       string(REGEX REPLACE "\n$" "" gnOutput "${gitOutput}")
       message("-- Git Commit ${gitOutput}")
       execute_process(
           COMMAND ${GIT_EXECUTABLE} show HEAD
           WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}
       )
   endif()
else()
   message(FATAL_ERROR "-- Git Commit found no commits for ${shas}")
endif()
