# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


cmake_minimum_required(VERSION 3.19)

project(Ninja
    DESCRIPTION "Ninja Build Tool"
    HOMEPAGE_URL "https://ninja-build.org/"
    LANGUAGES CXX C
)

set(NINJA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/ninja)
set(NINJA_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

find_package(Python3 REQUIRED)

if(WIN32)
    set(NINJA_EXECUTABLE ninja.exe)
else()
    set(NINJA_EXECUTABLE ninja)
endif()

file(MAKE_DIRECTORY ${NINJA_BINARY_DIR})

add_custom_command(
    OUTPUT ${NINJA_EXECUTABLE}
    WORKING_DIRECTORY ${NINJA_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${NINJA_BINARY_DIR}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E chdir ${NINJA_BINARY_DIR}/$<CONFIG>
        ${Python3_EXECUTABLE} ${NINJA_SOURCE_DIR}/configure.py --bootstrap
    USES_TERMINAL
    VERBATIM
)
add_custom_target(Ninja ALL DEPENDS ${NINJA_EXECUTABLE})
install(FILES ${NINJA_BINARY_DIR}/$<CONFIG>/${NINJA_EXECUTABLE}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    DESTINATION bin
)

