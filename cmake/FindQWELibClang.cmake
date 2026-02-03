# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CLANG AND MACOS)
    set(Clang_EXECUTABLE ${CMAKE_OBJCXX_COMPILER})
elseif(CLANG)
    set(Clang_EXECUTABLE ${CMAKE_CXX_COMPILER})
else()
    find_program(Clang_EXECUTABLE NAMES clang-cl clang)
endif()

if(Clang_EXECUTABLE)
    if(NOT DEFINED QWELibClang_BIN_PATH)
        # Extract the base dir from the clang executable
        if(MSVC)
            set(CLANG_PRINT_PATH_COMMAND /clang:-print-prog-name=clang)
        elseif(LINUX)
            set(CLANG_PRINT_PATH_COMMAND -print-prog-name=clang-cpp)
        else()
            set(CLANG_PRINT_PATH_COMMAND -print-prog-name=clang)
        endif()
        execute_process(
            COMMAND ${Clang_EXECUTABLE} ${CLANG_PRINT_PATH_COMMAND}
            OUTPUT_VARIABLE clang_output
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(TO_CMAKE_PATH "${clang_output}" clang_output) # $base_path/bin/clang
        get_filename_component(clang_output "${clang_output}" DIRECTORY) # $base_path/bin

        set(QWELibClang_BIN_PATH "${clang_output}" CACHE INTERNAL "internal")
    endif()

    # Try to find the llvm-config executable, and extract the library location from it
    find_program(QWELibClang_LLVM_CONFIG_EXECUTABLE
        NAMES llvm-config
        PATHS ${QWELibClang_BIN_PATH}
        NO_DEFAULT_PATH)

    if (QWELibClang_LLVM_CONFIG_EXECUTABLE)
        execute_process(
            COMMAND ${QWELibClang_LLVM_CONFIG_EXECUTABLE} --libdir
            OUTPUT_VARIABLE llvm_config_output
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(TO_CMAKE_PATH "${llvm_config_output}" llvm_config_output)
        get_filename_component(QWELibClang_BASE_PATH "${llvm_config_output}" DIRECTORY CACHE INTERNAL "internal")
    else()
        # No llvm-config. Get the base path from the binary directory
        # This is the expected path for Windows and macOS
        get_filename_component(QWELibClang_BASE_PATH "${QWELibClang_BIN_PATH}" DIRECTORY CACHE INTERNAL "internal")
    endif()

    find_file(Libclang_LIBRARY
        NAMES libclang.dll libclang.dylib libclang.so
        PATHS ${llvm_config_output} ${QWELibClang_BIN_PATH} ${QWELibClang_BASE_PATH}/lib ${QWELibClang_BASE_PATH}/lib64
        NO_DEFAULT_PATH)

    get_filename_component(QWELibClang_LIBRARY_DIR "${Libclang_LIBRARY}" DIRECTORY CACHE INTERNAL "internal")

    execute_process(
        COMMAND ${Clang_EXECUTABLE} --version
        OUTPUT_VARIABLE ClangVersion_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" QWELibClang_VERSION "${ClangVersion_OUTPUT}")

    # Find runtime-path
    if(NOT DEFINED QWELibClang_RUNTIME_PATH)
        set(CLANG_PRINT_RUNTIME_DIR_COMMAND -print-runtime-dir)
        if (MSVC)
            # clang-cl does not accept the argument unless it's piped via /clang:
            set(CLANG_PRINT_RUNTIME_DIR_COMMAND /clang:-print-runtime-dir)
        endif()
        execute_process(
           COMMAND ${Clang_EXECUTABLE} ${CLANG_PRINT_RUNTIME_DIR_COMMAND}
           OUTPUT_VARIABLE clang_output
           ERROR_QUIET
           OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(TO_CMAKE_PATH "${clang_output}" clang_output)
        set(QWELibClang_RUNTIME_PATH "${clang_output}" CACHE INTERNAL "internal")
    endif()

    # Find resource-path
    if(NOT DEFINED QWELibClang_RESOURCE_PATH)
        set(CLANG_PRINT_RUNTIME_DIR_COMMAND -print-resource-dir)
        if (MSVC)
            # clang-cl does not accept the argument unless it's piped via /clang:
            set(CLANG_PRINT_RUNTIME_DIR_COMMAND /clang:-print-resource-dir)
        endif()
        execute_process(
           COMMAND ${Clang_EXECUTABLE} ${CLANG_PRINT_RUNTIME_DIR_COMMAND}
           OUTPUT_VARIABLE clang_output
           ERROR_QUIET
           OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(TO_CMAKE_PATH "${clang_output}" clang_output)
        set(QWELibClang_RESOURCE_PATH "${clang_output}" CACHE INTERNAL "internal")
    endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(QWELibClang
    REQUIRED_VARS QWELibClang_LIBRARY_DIR QWELibClang_BASE_PATH QWELibClang_BIN_PATH
    VERSION_VAR QWELibClang_VERSION
)

mark_as_advanced(QWELibClang_BIN_PATH)
mark_as_advanced(QWELibClang_BASE_PATH)
mark_as_advanced(QWELibClang_LIBRARY_DIR)
mark_as_advanced(QWELibClang_RUNTIME_PATH)
mark_as_advanced(QWELibClang_RESOURCE_PATH)
