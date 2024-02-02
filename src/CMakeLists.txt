# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

##
#  MAIN CONFIGURE
##

qt_internal_get_filename_path_mode(path_mode)

get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ${path_mode})
get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${PROJECT_BINARY_DIR}" ${path_mode})

# Note this is configure that does not belong to any module
qt_feature_module_begin(ONLY_EVALUATE_FEATURES)
# Enable printing of feature summary by forcing qt_configure_record_command
# to work in spite of ONLY_EVALUATE_FEATURES.
set(__QtFeature_only_evaluate_features OFF)
include(../configure.cmake)
qt_feature_module_end(ONLY_EVALUATE_FEATURES)

##
#  SUPPORT CHECK
##

# TODO: here we should return FATAL_ERROR and ask user to define
# FEATURE_qtwebenignebuild=OFF if one wants to only build qtpdf,
# however our ci setup is not flexible enough and configure step must succeed

if(NOT ${QtWebEngine_SUPPORT})
    add_custom_target(WebEngineErrorMessage ALL
        ${CMAKE_COMMAND} -E cmake_echo_color --red "QtWebEngine will not be built: ${QtWebEngine_ERROR}"
        COMMENT "Check QtWebEngine support"
        VERBATIM
    )
    message("-- Support check for QtWebEngine failed: ${QtWebEngine_ERROR}")
    set(QT_FEATURE_qtwebengine_build OFF CACHE BOOL "Build QtWebEngine Modules" FORCE)
    set(QT_FEATURE_qtwebengine_core_build OFF CACHE BOOL "Build QtWebEngineCore" FORCE)
    set(QT_FEATURE_qtwebengine_widgets_build OFF CACHE BOOL "Build QtWebEngineWidgets" FORCE)
    set(QT_FEATURE_qtwebengine_quick_build OFF CACHE BOOL "Build QtWebEngineQuick" FORCE)

endif()

if(NOT ${QtPdf_SUPPORT})
    add_custom_target(PdfErrorMessage ALL
        ${CMAKE_COMMAND} -E cmake_echo_color --red "QtPdf will not be built: ${QtPdf_ERROR}"
        COMMENT "Check QtPdf support"
        VERBATIM
    )
    message("-- Support check for QtPdf failed: ${QtPdf_ERROR}")
    set(QT_FEATURE_qtpdf_build OFF CACHE BOOL "Build QtPdf" FORCE)
    set(QT_FEATURE_qtpdf_widgets_build OFF CACHE BOOL "Build QtPdfWidgets" FORCE)
    set(QT_FEATURE_qtpdf_quick_build OFF CACHE BOOL "Build QtPdfQuick" FORCE)
endif()

if(NOT QT_FEATURE_qtwebengine_build AND NOT QT_FEATURE_qtpdf_build)
    return()
endif()

# Upgrade CMake policies to the minimum supported version.
cmake_minimum_required(VERSION ${QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_BUILDING_WEBENGINE})

##
#  MODULES
##

if(QT_FEATURE_qtwebengine_core_build)
   add_subdirectory(core)
   add_subdirectory(process)
endif()
if(QT_FEATURE_qtwebengine_widgets_build)
   add_subdirectory(webenginewidgets)
endif()
if(QT_FEATURE_qtwebengine_quick_build)
   add_subdirectory(webenginequick)
endif()

if(QT_FEATURE_qtpdf_build)
   add_subdirectory(pdf)
   # keep log order, pdf build after webengine
   if(QT_FEATURE_qtwebengine_core_build)
       add_dependencies(run_pdf_NinjaReady WebEngineCore)
   endif()
   if(QT_FEATURE_qtwebengine_widgets_build)
       add_dependencies(run_pdf_NinjaReady WebEngineWidgets)
   endif()
   if(QT_FEATURE_qtwebengine_quick_build)
       add_dependencies(run_pdf_NinjaReady WebEngineQuick)
   endif()
   if(QT_FEATURE_qtpdf_widgets_build)
       add_subdirectory(pdfwidgets)
   endif()
   if(QT_FEATURE_qtpdf_quick_build)
       add_subdirectory(pdfquick)
   endif()
endif()

##
#  NINJA PROJECT
##

set(installDir ${PROJECT_BINARY_DIR}/install)

if(NOT Ninja_FOUND)
    externalproject_add(ninja
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/ninja
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/ninja
        INSTALL_DIR ${installDir}
        PREFIX      ninja
        USES_TERMINAL_BUILD TRUE
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
    )
    if(QT_FEATURE_qtwebengine_core_build)
        add_dependencies(run_core_NinjaReady ninja)
    endif()
    if(QT_FEATURE_qtpdf_build)
        add_dependencies(run_pdf_NinjaReady ninja)
    endif()
endif()

##
#  GN PROJECT
##

if(CMAKE_CROSSCOMPILING AND NOT Gn_FOUND)
   message(FATAL_ERROR "No gn found for cross-compilation")
endif()

if(NOT Gn_FOUND)
    externalproject_add(gn
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/gn
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/gn
        INSTALL_DIR ${installDir}
        PREFIX      gn
        USES_TERMINAL_BUILD TRUE
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
                   -DWEBENGINE_ROOT_BUILD_DIR=${PROJECT_BINARY_DIR}
                   -DQT_ALLOW_SYMLINK_IN_PATHS=${QT_ALLOW_SYMLINK_IN_PATHS}
    )
    if(QT_FEATURE_qtwebengine_core_build)
        add_dependencies(run_core_GnReady gn)
    endif()
    if(QT_FEATURE_qtpdf_build)
        add_dependencies(run_pdf_GnReady gn)
    endif()
endif()

if(NOT Gn_FOUND OR Gn_EXECUTABLE MATCHES "^${installDir}")
    set(INSTALL_GN 1 CACHE INTERNAL "")
endif()

##
#  HOST PROJECT
##

if(CMAKE_CROSSCOMPILING AND NOT IOS AND NOT MACOS)

    if(NOT Gn_FOUND)
        message(FATAL_ERROR "\nHost gn not found - cross compilation not possible")
    endif()

    externalproject_add(hostBuild
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/host
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/host
        PREFIX      host
        USES_TERMINAL_BUILD TRUE
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${QT_HOST_PATH_CMAKE_DIR}/Qt6/qt.toolchain.cmake
                   -DQT_USE_ORIGINAL_COMPILER=ON
                   -DWEBENGINE_ROOT_BUILD_DIR=${PROJECT_BINARY_DIR}
                   -DWEBENGINE_ROOT_SOURCE_DIR=${WEBENGINE_ROOT_SOURCE_DIR}
                   -DGN_TARGET_CPU=${TEST_architecture_arch}
                   -DCMAKE_C_FLAGS=
                   -DCMAKE_CXX_FLAGS=
                   -DQT_FEATURE_qtwebengine_build=${QT_FEATURE_qtwebengine_build}
                   -DQT_FEATURE_qtpdf_build=${QT_FEATURE_qtpdf_build}
    )
    if(QT_FEATURE_qtwebengine_build)
        add_dependencies(run_core_GnReady hostBuild)
    endif()
    if(QT_FEATURE_qtpdf_build)
        add_dependencies(run_pdf_GnReady hostBuild)
    endif()
endif()

# install gn for cross build
if((LINUX OR MACOS) AND INSTALL_GN)
    get_install_config(installConfig)
    install(
        PROGRAMS ${installDir}/bin/gn
        CONFIGURATIONS ${installConfig}
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
    )
    if(NOT QT_WILL_INSTALL)
        add_custom_target(copy-gn ALL DEPENDS
            ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/gn
        )
        if(Gn_FOUND)
            set(copyDep ${Gn_EXECUTABLE})
        else()
            set(copyDep gn)
        endif()
        add_custom_command(
            OUTPUT ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/gn
            COMMAND ${CMAKE_COMMAND} -E copy ${installDir}/bin/gn
            ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}
            DEPENDS ${copyDep}
            USES_TERMINAL
        )
    endif()
endif()

