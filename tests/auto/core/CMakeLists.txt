# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qwebenginecookiestore)
add_subdirectory(qwebengineloadinginfo)
add_subdirectory(qwebenginesettings)
if(QT_FEATURE_ssl)
    # only tests doh, and requires ssl
    add_subdirectory(qwebengineglobalsettings)
endif()
add_subdirectory(qwebengineurlrequestinterceptor)
add_subdirectory(qwebengineurlrequestjob)
add_subdirectory(origins)
add_subdirectory(devtools)
add_subdirectory(getdomainandregistry)
add_subdirectory(qtversion)

if(QT_FEATURE_ssl)
    add_subdirectory(qwebengineclientcertificatestore)
    add_subdirectory(certificateerror)
endif()

if(QT_FEATURE_webenginedriver)
    add_subdirectory(webenginedriver)
endif()
