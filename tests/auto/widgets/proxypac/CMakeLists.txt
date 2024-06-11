# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_proxypac_file
    SOURCES
        tst_proxypac.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
)

if(WIN32)
    get_filename_component(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
    set(fileEnvArg "--proxy-pac-url=\"file:///${SOURCE_DIR}/proxy.pac\"")
elseif(LINUX AND CMAKE_CROSSCOMPILING)
    set(fileEnvArg "--single-process --no-sandbox --proxy-pac-url=\"file://${CMAKE_CURRENT_LIST_DIR}/proxy.pac\"")
else()
    set(fileEnvArg "--proxy-pac-url=\"file://${CMAKE_CURRENT_LIST_DIR}/proxy.pac\"")
endif()

set_tests_properties(tst_proxypac_file PROPERTIES
    ENVIRONMENT QTWEBENGINE_CHROMIUM_FLAGS=${fileEnvArg}
)

if(NOT (LINUX AND CMAKE_CROSSCOMPILING))
    set(fileEnvArg "--single-process ${fileEnvArg}")

    qt_internal_add_test(tst_proxypac_single_process
        SOURCES
            tst_proxypac.cpp
        LIBRARIES
            Qt::WebEngineCore
            Test::HttpServer
    )

    set_tests_properties(tst_proxypac_single_process PROPERTIES
        ENVIRONMENT QTWEBENGINE_CHROMIUM_FLAGS=${fileEnvArg}
    )
endif()

qt_internal_add_test(tst_proxypac_qrc
    SOURCES
        tst_proxypac.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
)

if(LINUX AND CMAKE_CROSSCOMPILING)
    set(qrcEnvArg "--single-process --no-sandbox --proxy-pac-url=\"qrc:///proxy.pac\"")
else()
    set(qrcEnvArg "--proxy-pac-url=\"qrc:///proxy.pac\"")
endif()

set_tests_properties(tst_proxypac_qrc PROPERTIES
    ENVIRONMENT QTWEBENGINE_CHROMIUM_FLAGS=${qrcEnvArg}
)

qt_internal_add_resource(tst_proxypac_qrc "proxypac"
    PREFIX "/"
    FILES "proxy.pac"
)
