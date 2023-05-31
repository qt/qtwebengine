# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT TARGET Test::HttpServer)

   add_library(httpserver STATIC
      ${CMAKE_CURRENT_LIST_DIR}/httpreqrep.cpp
      ${CMAKE_CURRENT_LIST_DIR}/httpreqrep.h
      ${CMAKE_CURRENT_LIST_DIR}/httpserver.cpp
      ${CMAKE_CURRENT_LIST_DIR}/httpserver.h
      ${CMAKE_CURRENT_LIST_DIR}/proxy_server.h
      ${CMAKE_CURRENT_LIST_DIR}/proxy_server.cpp
   )

   # moc binary might not exist in case of top level build
   qt_autogen_tools(httpserver ENABLE_AUTOGEN_TOOLS "moc")

   if(QT_FEATURE_ssl)
      target_sources(httpserver INTERFACE ${CMAKE_CURRENT_LIST_DIR}/httpsserver.h)
   endif()

   add_library(Test::HttpServer ALIAS httpserver)

   target_include_directories(httpserver INTERFACE
       $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
   )

   target_link_libraries(httpserver PUBLIC
      Qt::Core
      Qt::Network
   )

   get_filename_component(SERVER_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
   target_compile_definitions(httpserver PRIVATE
       SERVER_SOURCE_DIR="${SERVER_SOURCE_DIR}"
   )

   set_target_properties(httpserver PROPERTIES
       SHARED_DATA "${CMAKE_CURRENT_LIST_DIR}/data"
   )
endif()
