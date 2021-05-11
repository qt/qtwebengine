cmake_minimum_required(VERSION 3.19)

include(.cmake.conf)
include(ExternalProject)
include(cmake/Functions.cmake)

project(QtWebEngineRepoSuperBuild
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt WebEngine Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

option(FORCE_TEST_RUN "Forces test run after compilation" FALSE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)

# coin configures 'just' for test build
if(QT_BUILD_STANDALONE_TESTS)
    qt_build_repo()
    return()
endif()

set(installDir ${CMAKE_CURRENT_BINARY_DIR}/install)

### FEATURES

qt_build_repo_begin()
qt_feature_module_begin(ONLY_EVALUATE_FEATURES)
# Enable printing of feature summary by forcing qt_configure_record_command
# to work in spite of ONLY_EVALUATE_FEATURES.
set(__QtFeature_only_evaluate_features OFF)
include(configure.cmake)
include(src/core/api/configure.cmake)
qt_feature_module_end(ONLY_EVALUATE_FEATURES)

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
    # coin missing artifacts workaround
    install(FILES coin-bug-707 DESTINATION ${INSTALL_DATADIR})
    return()
endif()

add_subdirectory(src/core/api)
add_subdirectory(src/process)
add_subdirectory(src/webenginewidgets)
add_subdirectory(src/webenginequick)
qt_build_repo_end()

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

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/qtbase/lib/cmake/Qt6/qt.toolchain.cmake")
    list(APPEND libsCmakeArgs
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
    )
endif()

list(TRANSFORM featureList PREPEND "-D")
list(APPEND libsCmakeArgs
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DWEBENGINE_ROOT_BUILD_PATH=${CMAKE_CURRENT_BINARY_DIR}"
        "-DWEBENGINE_ROOT_SOURCE_DIR=${WEBENGINE_ROOT_SOURCE_DIR}"
        "-DWEBENGINE_REPO_BUILD=TRUE"
        "${featureList}"
)

if(LINUX)
    set(cmd "${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/src")
    file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/build.sh CONTENT "ulimit -n 4096 && ${cmd}\n")
    set(coinBug699 COMMAND bash build.sh)
else()
    if ("${CMAKE_DEFAULT_CONFIGS}" STREQUAL "all")
        set(coinBug699 COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/src)
    else()
        set(coinBug699 COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/src --config $<CONFIG>)
    endif()
endif()

externalproject_add(libs
    SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src
    BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/src
    PREFIX      libs
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
    BUILD_ALWAYS TRUE
    CMAKE_ARGS  ${libsCmakeArgs}
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Starting $<CONFIG> build"
    ${coinBug699}
    COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"
    # TODO: this is aworkaround for broken qt_lib_webengine*.pri files,
    # coin does the in-source build and later calls qmake to build examples
    # and this leftover file gets sucked instead of install or src/mkspecs,
    # simply remove it, we need to implment a dry run in qt base to fix it.
    COMMAND ${CMAKE_COMMAND} -E remove_directory ../mkspecs
)

if(QT_BUILD_EXAMPLES AND BUILD_SHARED_LIBS)
    externalproject_add(examples
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/examples
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/examples
        PREFIX      examples
        USES_TERMINAL_BUILD ON
        BUILD_ALWAYS TRUE
        CMAKE_ARGS  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                    "${featureList}"
    )
    externalproject_add_stepdependencies(examples install libs)
endif()

if(FORCE_TEST_RUN OR QT_BUILD_TESTS)
     find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS QuickTest Test)
     unset(testRunCommand)
     if(FORCE_TEST_RUN)
        set(testRunCommand TEST_COMMAND ctest)
     endif()
     externalproject_add(tests
         SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/tests
         BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/tests
         PREFIX      tests
         USES_TERMINAL_BUILD ON
         USES_TERMINAL_TEST ON
         BUILD_ALWAYS TRUE
         CMAKE_ARGS  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                 "${featureList}"
         INSTALL_COMMAND ""
         ${testRunCommand}
    )
    externalproject_add_stepdependencies(tests install libs)
    if(QT_BUILD_EXAMPLES AND BUILD_SHARED_LIBS)
       externalproject_add_stepdependencies(tests install examples)
    endif()
endif()

if(NOT Gn_FOUND)
    externalproject_add_stepdependencies(libs install gn)
endif()

if(NOT Ninja_FOUND)
    externalproject_add_stepdependencies(gn install ninja)
endif()

if(NOT QT_FEATURE_framework)
    # install resources (again), coin calls install with DESTDIR
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/translations/qtwebengine_locales
        DESTINATION ${CMAKE_INSTALL_PREFIX}/translations
    )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
endif()
