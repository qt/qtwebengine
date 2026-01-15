# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(Rustc_EXECUTABLE NAMES rustc)
execute_process(
    COMMAND ${Rustc_EXECUTABLE} --version
    OUTPUT_VARIABLE Rustc_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"  Rust_VERSION "${Rustc_OUTPUT}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Rust
    REQUIRED_VARS Rustc_EXECUTABLE
    VERSION_VAR Rust_VERSION
)

mark_as_advanced(Rustc_EXECUTABLE)

