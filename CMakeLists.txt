cmake_minimum_required(VERSION 3.19)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)

project(QtWebEngine
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "QtWebEngine and QtPdf modules"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS
    Gui Widgets Network OpenGL Quick Qml PrintSupport
    WebChannel Positioning QuickControls2 Test QuickWidgets QuickTest WebSockets Designer
)

if(MATRIX_BUILD AND NOT MATRIX_SUBBUILD AND NOT QT_SUPERBUILD)
    add_build(webengine_webrtc OFF)
    add_build(webengine_proprietary_codecs ON)
    add_build(webengine_printing_and_pdf OFF)
    add_build(webengine_extensions OFF)
    add_build(webengine_spellchecker OFF)
    add_build(qtwebengine_build OFF)
    return()
endif()

set(WEBENGINE_MODULE_BUILD TRUE)
qt_build_repo()
