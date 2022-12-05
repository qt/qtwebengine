# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineclientcertificatestore
    SOURCES
        tst_qwebengineclientcertificatestore.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Test::Util
)

set(tst_qwebengineclientcertificatestore_resource_files
    "resources/certificate.crt"
    "resources/certificate1.crt"
    "resources/privatekey.key"
    "resources/privatekey1.key"
    "resources/server.pem"
    "resources/server.key"
    "resources/client.pem"
    "resources/client.key"
    "resources/client2.pem"
    "resources/client2.key"
    "resources/ca.pem"
)

qt_internal_add_resource(tst_qwebengineclientcertificatestore "tst_qwebengineclientcertificatestore"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineclientcertificatestore_resource_files}
)

if(LINUX AND NOT CMAKE_CROSSCOMPILING)

    get_filename_component(homePath $ENV{HOME} ABSOLUTE)

    find_program(pk12util_EXECUTABLE NAMES pk12util)
    find_program(certutil_EXECUTABLE NAMES certutil)

    if(pk12util_EXECUTABLE AND certutil_EXECUTABLE)
        add_custom_command(
            DEPENDS resources/client2.p12
            COMMAND test -e "${homePath}/.pki/nssdb" || ${CMAKE_COMMAND} -E make_directory
                "${homePath}/.pki/nssdb"
            COMMAND test -e "${homePath}/.pki/nssdb/cert9.db" || ${certutil_EXECUTABLE}
                -N --empty-password -d sql:${homePath}/.pki/nssdb
            COMMAND test -e "${homePath}/.pki/nssdb/cert9.db" && ${pk12util_EXECUTABLE}
                -d sql:${homePath}/.pki/nssdb
                -i "${CMAKE_CURRENT_LIST_DIR}/resources/client2.p12"
                -W ""
            COMMAND ${CMAKE_COMMAND} -E touch pk12util.stamp
            OUTPUT pk12util.stamp
            VERBATIM
            USES_TERMINAL
        )
        add_custom_target(
            add-user-personal-certificate
            DEPENDS pk12util.stamp
    )
    qt_internal_extend_target(tst_qwebengineclientcertificatestore DEFINES TEST_NSS)
    add_dependencies(tst_qwebengineclientcertificatestore add-user-personal-certificate)
    endif()

    find_program(certutil_EXECUTABLE NAMES certutil)

    if(certutil_EXECUTABLE)
        add_custom_target(remove-user-personal-certificate
            COMMAND ${CMAKE_COMMAND} -E remove pk12util.stamp
            COMMAND ${certutil_EXECUTABLE}
                -d sql:"${homePath}/.pki/nssdb"
                -D
                -n qwebengineclientcertificatestore
        )
    endif()
endif()

