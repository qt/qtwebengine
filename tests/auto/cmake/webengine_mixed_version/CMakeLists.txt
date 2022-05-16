# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(webengine_mixed_version LANGUAGES CXX)

set(failed FALSE)

function(assertFalse condition)
if(${condition})
    message(WARNING "Test check failed: ${condition} was expected false, was ${${condition}}")
    set(failed TRUE PARENT_SCOPE)
endif()
endfunction()

function(assertTrue condition)
if(NOT ${condition})
    message(WARNING "Test check failed: ${condition} was expected true, was ${${condition}}")
    set(failed TRUE PARENT_SCOPE)
endif()
endfunction()

string(REPLACE "." ";" projectVersion ${WEBENGINE_VERSION})
list(GET projectVersion 0 projectVersionMajor)
list(GET projectVersion 1 projectVersionMinor)
list(GET projectVersion 2 projectVersionPatch)


find_package(Qt6WebEngineCore ${WEBENGINE_VERSION} QUIET)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6WebEngineWidgets ${WEBENGINE_VERSION} QUIET)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${WEBENGINE_VERSION} COMPONENTS WebEngineCore WebEngineWidgets)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${WEBENGINE_VERSION} EXACT COMPONENTS WebEngineCore WebEngineWidgets)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor} QUIET
    COMPONENTS WebEngineWidgets WebEngineCore)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor}.99 QUIET EXACT
    COMPONENTS WebEngineWidgets WebEngineCore)
assertFalse(Qt6_FOUND)
unset(Qt6_FOUND)

set(QT_NO_PACKAGE_VERSION_CHECK TRUE)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor} COMPONENTS Gui WebEngineCore QUIET)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor} COMPONENTS WebEngineCore Gui QUIET)
assertTrue(Qt6_FOUND)
unset(Qt6_FOUND)

set(QT_NO_PACKAGE_VERSION_CHECK FALSE)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor} COMPONENTS Gui WebEngineCore QUIET)
assertFalse(Qt6_FOUND)
unset(Qt6_FOUND)

find_package(Qt6 ${projectVersionMajor}.${projectVersionMinor} COMPONENTS WebEngineCore Gui QUIET)
assertFalse(Qt6_FOUND)
unset(Qt6_FOUND)


if(failed)
    message(FATAL_ERROR "One or more tests failed")
endif()
