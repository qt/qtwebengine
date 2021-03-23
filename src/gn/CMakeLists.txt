
cmake_minimum_required(VERSION 3.19)

project(Gn
    DESCRIPTION "Meta-build system"
    HOMEPAGE_URL "https://gn.googlesource.com/gn/"
    LANGUAGES CXX C
)

set(GN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gn)
set(GN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

find_package(Python2 REQUIRED)
find_package(Ninja 1.7.2 REQUIRED)

if(WIN32)
    set(GN_EXECUTABLE gn.exe)
else()
    set(GN_EXECUTABLE gn)
endif()

file(MAKE_DIRECTORY ${GN_BINARY_DIR})
add_custom_command(
    OUTPUT ${GN_EXECUTABLE}
    WORKING_DIRECTORY ${GN_BINARY_DIR}
    COMMAND ${Python2_EXECUTABLE} ${GN_SOURCE_DIR}/build/gen.py
        --no-last-commit-position
        --out-path ${GN_BINARY_DIR}/$<CONFIG>
        --cc ${CMAKE_C_COMPILER}
        --cxx ${CMAKE_CXX_COMPILER}
        --ld ${CMAKE_CXX_COMPILER}
        $<$<PLATFORM_ID:Darwin>:--isysroot>
        $<$<PLATFORM_ID:Darwin>:${CMAKE_OSX_SYSROOT}>
    COMMAND Ninja::ninja -C ${GN_BINARY_DIR}/$<CONFIG> ${GN_EXECUTABLE}
    VERBATIM
    USES_TERMINAL
    COMMAND_EXPAND_LISTS
)
add_custom_target(Gn ALL DEPENDS ${GN_EXECUTABLE})
install(FILES ${GN_BINARY_DIR}/$<CONFIG>/${GN_EXECUTABLE}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        DESTINATION bin
)
