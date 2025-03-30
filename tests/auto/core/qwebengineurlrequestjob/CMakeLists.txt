# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineurlrequestjob
    SOURCES
        tst_qwebengineurlrequestjob.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::Util
)

# Resources:
set(tst_qwebengineurlrequestjob_resource_files
    "additionalResponseHeadersScript.html"
    "requestBodyScript.html"
)

qt_add_resources(tst_qwebengineurlrequestjob "tst_qwebengineurlrequestjob"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineurlrequestjob_resource_files}
)
