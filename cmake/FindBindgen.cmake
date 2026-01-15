# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(Bindgen_EXECUTABLE NAMES bindgen)

execute_process(
    COMMAND ${Bindgen_EXECUTABLE} --version
    OUTPUT_VARIABLE Bindgen_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"  Bindgen_VERSION "${Bindgen_OUTPUT}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Bindgen
    REQUIRED_VARS Bindgen_EXECUTABLE
    VERSION_VAR Bindgen_VERSION
)

mark_as_advanced(Bindgen_EXECUTABLE)

