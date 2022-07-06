# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

add_subdirectory(qwebenginecookiestore)
add_subdirectory(qwebenginesettings)
add_subdirectory(qwebengineurlrequestinterceptor)
add_subdirectory(origins)
add_subdirectory(devtools)

if(QT_FEATURE_ssl)
    add_subdirectory(qwebengineclientcertificatestore)
    add_subdirectory(certificateerror)
endif()
