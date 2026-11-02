# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(QWERustfmt_EXECUTABLE NAMES rustfmt)

execute_process(
    COMMAND ${QWERustfmt_EXECUTABLE} --version
    OUTPUT_VARIABLE QWERustfmt_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"  QWERustfmt_VERSION "${QWERustfmt_OUTPUT}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(QWERustfmt
    REQUIRED_VARS QWERustfmt_EXECUTABLE
    VERSION_VAR QWERustfmt_VERSION
)

mark_as_advanced(QWERustfmt_EXECUTABLE)
