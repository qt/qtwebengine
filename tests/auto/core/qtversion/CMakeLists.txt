# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qtversion
    SOURCES
        tst_qtversion.cpp
    LIBRARIES
        Qt::WebEngineCore
)

# simply embed the CHROMIUM_VERSION file, so the test works accross coin nodes
get_filename_component(webengineRootSourceDir "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
configure_file(
    "${webengineRootSourceDir}/CHROMIUM_VERSION"
    "${CMAKE_CURRENT_BINARY_DIR}/CHROMIUM_VERSION"
    COPYONLY
)
qt_internal_add_resource(tst_qtversion "tst_qtversion"
    PREFIX
        "/"
    BASE
        "${CMAKE_CURRENT_BINARY_DIR}"
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/CHROMIUM_VERSION"
)
