# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(QWERustc_EXECUTABLE NAMES rustc)
execute_process(
    COMMAND ${QWERustc_EXECUTABLE} --version
    OUTPUT_VARIABLE QWERustc_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"  QWERust_VERSION "${QWERustc_OUTPUT}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(QWERust
    REQUIRED_VARS QWERustc_EXECUTABLE
    VERSION_VAR QWERust_VERSION
)

mark_as_advanced(QWERustc_EXECUTABLE)
