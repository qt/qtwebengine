# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qwebengineurlrequestjob
    SOURCES
        tst_qwebengineurlrequestjob.cpp
    LIBRARIES
        Qt::WebEngineCore
)

# Resources:
set(tst_qwebengineurlrequestjob_resource_files
    "additionalResponseHeadersScript.html"
)

qt_add_resources(tst_qwebengineurlrequestjob "tst_qwebengineurlrequestjob"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineurlrequestjob_resource_files}
)
