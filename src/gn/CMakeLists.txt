
cmake_minimum_required(VERSION 3.19)

project(Gn
    DESCRIPTION "Meta-build system"
    HOMEPAGE_URL "https://gn.googlesource.com/gn/"
    LANGUAGES CXX C
)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
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
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Use lld-link instead of clang-cl.
        set(GN_LINKER ${CMAKE_LINKER})
    endif()
else()
    set(GN_EXECUTABLE gn)
endif()

file(MAKE_DIRECTORY ${GN_BINARY_DIR})
add_custom_command(
    OUTPUT ${GN_EXECUTABLE}
    WORKING_DIRECTORY ${GN_BINARY_DIR}
    COMMAND ${Python3_EXECUTABLE} ${GN_SOURCE_DIR}/build/gen.py
        --no-last-commit-position
        --out-path ${GN_BINARY_DIR}/$<CONFIG>
        --cc ${CMAKE_C_COMPILER}
        --cxx ${CMAKE_CXX_COMPILER}
        --ld ${GN_LINKER}
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
