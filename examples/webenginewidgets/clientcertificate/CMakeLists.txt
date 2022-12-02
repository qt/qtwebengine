# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(clientcertificate LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/clientcertificate")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(server
    server.cpp
)

qt_add_executable(client
    client.cpp
)

set_target_properties(client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

qt_add_resources(client "client"
    PREFIX
        "/"
    FILES
       "resources/client.pem"
       "resources/client.key"
)

qt_add_resources(server "server"
    PREFIX
        "/"
    FILES
       "resources/server.pem"
       "resources/server.key"
       "resources/ca.pem"
)

target_link_libraries(client PUBLIC
    Qt::WebEngineWidgets
)

target_link_libraries(server PUBLIC
    Qt::Core
    Qt::Network
)

install(TARGETS server client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
