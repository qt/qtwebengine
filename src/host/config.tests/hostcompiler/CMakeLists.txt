# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(arch LANGUAGES CXX)

add_executable(host_compiler_test)
set_property(TARGET host_compiler_test PROPERTY MACOSX_BUNDLE FALSE)
target_sources(host_compiler_test PRIVATE main.cpp)
target_compile_options(host_compiler_test PRIVATE -m32)
target_link_options(host_compiler_test PRIVATE -m32)
