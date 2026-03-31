# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(QWEBindgen_EXECUTABLE NAMES bindgen)

execute_process(
    COMMAND ${QWEBindgen_EXECUTABLE} --version
    OUTPUT_VARIABLE QWEBindgen_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"  QWEBindgen_VERSION "${QWEBindgen_OUTPUT}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(QWEBindgen
    REQUIRED_VARS QWEBindgen_EXECUTABLE
    VERSION_VAR QWEBindgen_VERSION
)

mark_as_advanced(QWEBindgen_EXECUTABLE)
