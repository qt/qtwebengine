# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Require the Qt (not WebEngine) minimum supported CMake version.
# Another WebEngine-specific version check will be done in configure.cmake
# and reported in the configure summary.
cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)
include(cmake/QtGnTargetHelpers.cmake)
include(cmake/QtGnConfigHelpers.cmake)
include(cmake/QtConfigureHelpers.cmake)
include(cmake/QtWebEngineSbomHelpers.cmake)
include(cmake/QtToolchainHelpers.cmake)

project(QtWebEngineDummy)
find_package(Qt6 6.5 CONFIG REQUIRED COMPONENTS BuildInternals Core)

# Sepcial case of just doing gn build
if(DEFINED BUILD_ONLY_GN)
    qt_webengine_build_and_install_gn()
    return()
endif()

project(QtWebEngine
    VERSION ${Qt6Core_VERSION}
    DESCRIPTION "QtWebEngine and QtPdf modules"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS
    Gui Widgets Network OpenGL Quick Qml PrintSupport
    WebChannel WebChannelQuick Positioning QuickControls2
    Test QuickWidgets QuickTest WebSockets Designer
    JpegPrivate PngPrivate HarfbuzzPrivate FreetypePrivate ZlibPrivate
    HttpServer Svg
)

if(MATRIX_BUILD AND NOT MATRIX_SUBBUILD AND NOT QT_SUPERBUILD)
    qt_webengine_add_build(webengine_webrtc OFF)
    qt_webengine_add_build(webengine_proprietary_codecs ON)
    qt_webengine_add_build(webengine_printing_and_pdf OFF)
    qt_webengine_add_build(webengine_extensions OFF)
    qt_webengine_add_build(webengine_spellchecker OFF)
    qt_webengine_add_build(qtwebengine_build OFF)
    return()
endif()

# Don't auto create the SBOM projects, they will be manually created
# for each sub-project.
set(QT_SKIP_SBOM_AUTO_PROJECT TRUE)
qt_build_repo()

# Coin sanity check after qt_build_repo report is printed

qt_webengine_configure_check_coin(coin.nodes
    "${QT_CONFIGURE_CHECK_qtwebengine_build}"
    "${QT_CONFIGURE_CHECK_qtpdf_build}"
)
