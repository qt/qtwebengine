# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

cmake_minimum_required(VERSION 3.18)
project(minimal LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Network)

include(util.cmake)
