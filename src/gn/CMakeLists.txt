# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


cmake_minimum_required(VERSION 3.19)

project(Gn
    DESCRIPTION "Meta-build system"
    HOMEPAGE_URL "https://gn.googlesource.com/gn/"
    LANGUAGES CXX C
)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    set(path_mode REALPATH)
    if(APPLE AND QT_ALLOW_SYMLINK_IN_PATHS)
        set(path_mode ABSOLUTE)
    endif()

    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ${path_mode})
endif()

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)

set(GN_SOURCE_DIR ${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/gn)
set(GN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(GN_LINKER ${CMAKE_CXX_COMPILER})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBENGINE_ROOT_SOURCE_DIR}/cmake")

find_package(Python3 REQUIRED)
find_package(Ninja 1.7.2 REQUIRED)

if(WIN32)
    set(GN_EXECUTABLE gn.exe)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MINGW)
        # Use lld-link instead of clang-cl.
        set(GN_LINKER ${CMAKE_LINKER})
    endif()
else()
    set(GN_EXECUTABLE gn)
endif()

file(MAKE_DIRECTORY ${GN_BINARY_DIR})

if((UNIX AND NOT APPLE) AND
    (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR
    CMAKE_CXX_COMPILER_ID STREQUAL Clang))
    set(platform linux)
elseif(MSVC)
    set(platform msvc)
elseif(MINGW)
    set(platform mingw)
elseif(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    set(platform darwin)
else()
    message(FATAL_ERROR "Unsupported gn platform !")
endif()

add_custom_command(
    OUTPUT ${GN_EXECUTABLE}
    WORKING_DIRECTORY ${GN_BINARY_DIR}
    COMMAND ${Python3_EXECUTABLE} ${GN_SOURCE_DIR}/build/gen.py
        --no-last-commit-position
        --out-path ${GN_BINARY_DIR}/$<CONFIG>
        --cc ${CMAKE_C_COMPILER}
        --cxx ${CMAKE_CXX_COMPILER}
        --ld ${GN_LINKER}
        --platform ${platform}
        --ar ${CMAKE_AR}
        --qt-version "${QT_REPO_MODULE_VERSION}.qtwebengine.qt.io"
        $<$<PLATFORM_ID:Darwin>:--isysroot>
        $<$<PLATFORM_ID:Darwin>:${CMAKE_OSX_SYSROOT}>
    COMMAND Ninja::ninja -C ${GN_BINARY_DIR}/$<CONFIG> ${GN_EXECUTABLE}
    VERBATIM
    USES_TERMINAL
    COMMAND_EXPAND_LISTS
)
add_custom_target(Gn ALL DEPENDS ${GN_EXECUTABLE})
install(PROGRAMS ${GN_BINARY_DIR}/$<CONFIG>/${GN_EXECUTABLE}
        DESTINATION bin
)
