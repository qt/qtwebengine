# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
#
# This script reads a response file containing a list of object files paths, and creates
# symlinks to those object files in a specified directory. It then writes a new response
# file containing the paths to the symlinks.
cmake_minimum_required(VERSION 3.19)

if(NOT INPUT_RESPONSE_FILE_PATH)
    message(FATAL_ERROR "INPUT_RESPONSE_FILE_PATH is not set")
endif()

if(NOT EXISTS "${INPUT_RESPONSE_FILE_PATH}")
    message(FATAL_ERROR "Input response file does not exist: '${INPUT_RESPONSE_FILE_PATH}'")
endif()

if(NOT OUTPUT_RESPONSE_FILE_PATH)
    message(FATAL_ERROR "OUTPUT_RESPONSE_FILE_PATH is not set")
endif()

if(NOT SYMLINK_BASE_DIR)
    message(FATAL_ERROR "SYMLINK_BASE_DIR is not set")
endif()

if(NOT OUTPUT_SYMLINKS_DIR)
    message(FATAL_ERROR "OUTPUT_SYMLINKS_DIR is not set")
endif()

file(READ "${INPUT_RESPONSE_FILE_PATH}" response_file_contents)

string(REPLACE "\n" ";" response_file_contents "${response_file_contents}")
string(REPLACE "\"" "" response_file_contents "${response_file_contents}")

set(new_response_file_contents "")

foreach(entry IN LISTS response_file_contents)
    if(entry STREQUAL "")
        continue()
    endif()

    # Get entry path based on base dir, to shorten the object file names.
    file(RELATIVE_PATH relative_entry_path "${SYMLINK_BASE_DIR}" "${entry}")

    # Replace slashes with underscores, to create a valid file name.
    string(REPLACE "/" "_" symlink_name "${relative_entry_path}")

    set(symlink_path "${OUTPUT_SYMLINKS_DIR}/${symlink_name}")

    file(CREATE_LINK "${entry}" "${symlink_path}" RESULT sym_result SYMBOLIC)
    if(NOT sym_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to create symlink from '${symlink_path}' to '${entry}': ${sym_result}")
    endif()

    list(APPEND new_response_file_contents "\"${symlink_path}\"")
endforeach()

list(JOIN new_response_file_contents "\n" new_response_file_contents)
file(WRITE "${OUTPUT_RESPONSE_FILE_PATH}" "${new_response_file_contents}")
