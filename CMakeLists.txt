# Require the Qt (not WebEngine) minimum supported CMake version.
# Another WebEngine-specific version check will be done in configure.cmake
# and reported in the configure summary.
cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)
include(src/core/api/Qt6WebEngineCoreMacros.cmake)

project(QtWebEngineDummy)
find_package(Qt6 6.2 CONFIG REQUIRED COMPONENTS BuildInternals Core)

project(QtWebEngine
    VERSION ${Qt6Core_VERSION}
    DESCRIPTION "QtWebEngine and QtPdf modules"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS
    Gui Widgets Network OpenGL Quick Qml PrintSupport
    WebChannel Positioning QuickControls2 Test QuickWidgets QuickTest WebSockets Designer
    JpegPrivate PngPrivate HarfbuzzPrivate FreetypePrivate ZlibPrivate
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

qt_build_repo()
