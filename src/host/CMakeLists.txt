# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
endif()
if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/../.." REALPATH)
endif()

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBENGINE_ROOT_SOURCE_DIR}/cmake")

project(QtWebEngineConfigure
    VERSION "${QT_REPO_MODULE_VERSION}"
    LANGUAGES CXX C)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
qt_internal_project_setup()

set(buildDir ${CMAKE_CURRENT_BINARY_DIR})
configure_gn_toolchain(host ${TEST_architecture_arch} ${TEST_architecture_arch}
    ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
    ${buildDir}/host_toolchain
)
get_v8_arch(GN_V8_HOST_CPU ${GN_TARGET_CPU} ${TEST_architecture_arch})
configure_gn_toolchain(v8 ${GN_V8_HOST_CPU} ${GN_TARGET_CPU}
    ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
    ${buildDir}/v8_toolchain)

if(QT_FEATURE_qtwebengine_build)
    install(FILES ${buildDir}/host_toolchain/BUILD.gn
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/src/core/host_toolchain
    )
    install(FILES ${buildDir}/v8_toolchain/BUILD.gn
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/src/core/v8_toolchain
    )
endif()
if(QT_FEATURE_qtpdf_build)
    install(FILES ${buildDir}/host_toolchain/BUILD.gn
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/src/pdf/host_toolchain
    )
    install(FILES ${buildDir}/v8_toolchain/BUILD.gn
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/src/pdf/v8_toolchain
    )
endif()

# TODO: this could be run as part of main configure with execute_process

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU AND TEST_architecture_arch STREQUAL "x86_64"
AND GN_TARGET_CPU STREQUAL "arm")
    try_compile(
        has32HostCompiler
        "${CMAKE_CURRENT_BINARY_DIR}/config.tests/hostcompiler"
        "${CMAKE_CURRENT_SOURCE_DIR}/config.tests/hostcompiler"
        hostcompiler
    )
    if(NOT has32HostCompiler)
        MESSAGE(FATAL_ERROR "Compiler does not support 32bit compilation")
    endif()
endif()
