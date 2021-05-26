cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
endif()

include(${WEBENGINE_ROOT_SOURCE_DIR}/.cmake.conf)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBENGINE_ROOT_SOURCE_DIR}/cmake")

assertRunAsTopLevelBuild(TRUE)

project(QtWebEngineRepo
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "QtWebEngineRepo for QtWebEngineCore, QtWebEngine, QtWebEngineWidgets, QtPdf modules"
    HOMEPAGE_URL "https://qt.io"
    LANGUAGES CXX C)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)

qt_build_repo_begin()
add_subdirectory(core)
add_subdirectory(process)
add_subdirectory(webenginewidgets)
add_subdirectory(webenginequick)
qt_build_repo_end()

