# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.18)
project(minimal LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Network)

include(httpserver.cmake)
