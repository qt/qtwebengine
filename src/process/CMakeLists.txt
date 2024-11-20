# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    qt_internal_get_filename_path_mode(path_mode)

    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ${path_mode})
endif()
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

find_package(Qt6 COMPONENTS Gui)

get_target_property(qtWebEngineProcessName WebEngineCore QTWEBENGINEPROCESS_NAME)
get_target_property(isFramework WebEngineCore FRAMEWORK)

if(isFramework)
  set(install_dir "${INSTALL_LIBDIR}/QtWebEngineCore.framework/Versions/A/Helpers")
else()
  set(install_dir "${INSTALL_LIBEXECDIR}")
endif()

qt_internal_add_app(${qtWebEngineProcessName}
    TARGET_DESCRIPTION "QtWebEngine internal process"
    INSTALL_DIR ${install_dir}
    INCLUDE_DIRECTORIES ../core
    SOURCES main.cpp
)

if(WIN32)
    target_sources(${qtWebEngineProcessName} PRIVATE support_win.cpp)
    target_sources(${qtWebEngineProcessName} PRIVATE QtWebEngineProcess.exe.manifest)
    target_link_libraries(${qtWebEngineProcessName} PRIVATE WebEngineCoreSandbox)
    # get libs rsp file, since cmake is not aware of PUBLIC libs for WebEngineCore
    get_target_property(libs_rsp WebEngineCore LIBS_RSP)
    target_link_options(${qtWebEngineProcessName} PRIVATE "@${libs_rsp}" "/STACK:0x800000")
endif()

if(MACOS)
    set_target_properties(${qtWebEngineProcessName}
        PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.qt-project.Qt.QtWebEngineProcess"
    )
endif()

qt_internal_extend_target(${qtWebEngineProcessName}
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::WebEngineCore
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineCorePrivate
)

qt_get_cmake_configurations(configs)
foreach(config ${configs})
    string(TOUPPER "${config}" config_upper)
    set(outputPath "${QT_BUILD_DIR}/${install_dir}")
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${outputPath}"
    )

    if("${config}" STREQUAL "Debug")
        set_target_properties(${qtWebEngineProcessName} PROPERTIES
            OUTPUT_NAME_DEBUG ${qtWebEngineProcessName}${CMAKE_DEBUG_POSTFIX}
        )
    endif()
endforeach()

if(isFramework)
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
        MACOSX_BUNDLE TRUE
        INSTALL_RPATH "@loader_path/../../../../../../../"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist.in"
    )

    target_sources(${qtWebEngineProcessName} PRIVATE QtWebEngineProcess.entitlements)
    set_source_files_properties(QtWebEngineProcess.entitlements PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    add_custom_command(TARGET ${qtWebEngineProcessName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_BUNDLE_DIR:WebEngineCore>/Versions/A/Helpers/${qtWebEngineProcessName}.app"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Helpers"
                "$<TARGET_BUNDLE_DIR:WebEngineCore>/Helpers"
        USES_TERMINAL
    )
endif()
