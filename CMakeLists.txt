cmake_minimum_required(VERSION 3.19)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)

project(QtWebEngineAndQtPdf
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "QtWebEngine and QtPdf modules"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS
    Gui Widgets Network OpenGL OpenGLWidgets Quick Qml PrintSupport
    WebChannel Positioning QuickControls2 Test QuickWidgets QuickTest WebSockets
)

qt_build_repo()
