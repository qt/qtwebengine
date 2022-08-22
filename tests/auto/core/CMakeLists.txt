# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qwebenginecookiestore)
add_subdirectory(qwebenginesettings)
add_subdirectory(qwebengineurlrequestinterceptor)
add_subdirectory(origins)
add_subdirectory(devtools)

if(QT_FEATURE_ssl)
    add_subdirectory(qwebengineclientcertificatestore)
    add_subdirectory(certificateerror)
endif()
