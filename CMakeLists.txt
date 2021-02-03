cmake_minimum_required(VERSION 3.18.0)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)

project(QtWebEngineSuperBuild
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt WebEngine Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)

set(installDir ${CMAKE_CURRENT_BINARY_DIR}/install)

### FEATURES

qt_build_repo_begin()
qt_feature_module_begin(ONLY_EVALUATE_FEATURES)
# Enable printing of feature summary by forcing qt_configure_record_command
# to work in spite of ONLY_EVALUATE_FEATURES.
set(__QtFeature_only_evaluate_features OFF)
include(configure.cmake)
qt_feature_module_end(ONLY_EVALUATE_FEATURES)
qt_build_repo_end()

if(NOT ${webEngineSupport})
    add_custom_target(WebEngineErrorMessage ALL
        ${CMAKE_COMMAND} -E cmake_echo_color --red "QtWebEngine will not be built: ${webEngineError}"
        COMMENT "Check QtWebEngine support")
    message("-- Support check for QtWebEngine failed: ${webEngineError}")
    set(QT_FEATURE_qtwebengine_build OFF CACHE BOOL "Build QtWebEngine" FORCE)
endif()

if(NOT ${pdfSupport})
    add_custom_target(PdfErrorMessage ALL
        ${CMAKE_COMMAND} -E cmake_echo_color --red "QtPdf will not be built: ${pdfError}"
        COMMENT "Check QtPdf support")
    message("-- Support check for QtPdf failed: ${pdfError}")
    set(QT_FEATURE_qtpdf_build OFF CACHE BOOL "Build QtPdf" FORCE)
endif()

if(NOT QT_FEATURE_qtwebengine_build AND NOT QT_FEATURE_qtpdf_build)
    return()
endif()

### NINJA

if(NOT Ninja_FOUND)
    externalproject_add(ninja
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src/ninja
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/ninja
        INSTALL_DIR ${installDir}
        PREFIX      ninja
        USES_TERMINAL_BUILD ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
)
endif()

### GN

if(NOT Gn_FOUND)
    externalproject_add(gn
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src/gn
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/gn
        INSTALL_DIR ${installDir}
        PREFIX      gn
        USES_TERMINAL_BUILD ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
                   -DWEBENGINE_ROOT_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
)
endif()

### LIBS

# on windows source path can be specified without hard drive letter,
# however we need that for gn use REALPATH
get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
get_qt_features(featureList webengine)
get_qt_features(featureList qtwebengine)

list(TRANSFORM featureList PREPEND "-D")
list(APPEND webenginecoreCmakeArgs
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DWEBENGINE_ROOT_BUILD_PATH=${CMAKE_CURRENT_BINARY_DIR}"
        "-DWEBENGINE_ROOT_SOURCE_DIR=${WEBENGINE_ROOT_SOURCE_DIR}"
        "${featureList}")

externalproject_add(libs
    SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src/core
    BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/src/core
    INSTALL_DIR ${installDir}
    PREFIX      libs
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
    CMAKE_ARGS  ${webenginecoreCmakeArgs}
)

if(NOT Gn_FOUND)
    externalproject_add_stepdependencies(libs install gn)
endif()

if(NOT Ninja_FOUND)
    externalproject_add_stepdependencies(gn install ninja)
endif()

