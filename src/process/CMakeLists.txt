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

qt_internal_add_executable(${qtWebEngineProcessName}
    NO_INSTALL
    SOURCES main.cpp
)

if(WIN32)
    target_sources(${qtWebEngineProcessName} PRIVATE support_win.cpp)
    target_sources(${qtWebEngineProcessName} PRIVATE QtWebEngineProcess.exe.manifest)
    target_link_libraries(${qtWebEngineProcessName} PRIVATE WebEngineCoreSandbox)
    set_property(TARGET ${qtWebEngineProcessName} PROPERTY WIN32_EXECUTABLE TRUE)
    # get libs rsp file, since cmake is not aware of PUBLIC libs for WebEngineCore
    get_target_property(libs_rsp WebEngineCore LIBS_RSP)
    target_link_options(${qtWebEngineProcessName} PRIVATE "@${libs_rsp}")
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

target_include_directories(${qtWebEngineProcessName} PRIVATE ../core)

qt_get_cmake_configurations(configs)

foreach(config ${configs})
    string(TOUPPER "${config}" config_upper)
    if(isFramework)
       set(outputPath
           "${QT_BUILD_DIR}/${INSTALL_LIBDIR}/QtWebEngineCore.framework/Versions/A/Helpers"
       )
    else()
       set(outputPath "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}")
    endif()
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${outputPath}"
    )
endforeach()

if(QT_FEATURE_debug_and_release)
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
        OUTPUT_NAME_DEBUG ${qtWebEngineProcessName}${CMAKE_DEBUG_POSTFIX}
    )
endif()

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
    install(TARGETS ${qtWebEngineProcessName}
        BUNDLE DESTINATION "${INSTALL_LIBDIR}/QtWebEngineCore.framework/Versions/A/Helpers"
        COMPONENT Runtime
    )
    qt_enable_separate_debug_info(${qtWebEngineProcessName}
        "${INSTALL_LIBDIR}" QT_EXECUTABLE
        DSYM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${INSTALL_LIBDIR}"
    )
else()
    qt_apply_rpaths(TARGET ${qtWebEngineProcessName} INSTALL_PATH "${INSTALL_LIBEXECDIR}" RELATIVE_RPATH)
    install(TARGETS ${qtWebEngineProcessName}
        CONFIGURATIONS ${configs}
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
    )
    qt_enable_separate_debug_info(${qtWebEngineProcessName} "${INSTALL_LIBEXECDIR}" QT_EXECUTABLE)
    qt_internal_install_pdb_files(${qtWebEngineProcessName} "${INSTALL_LIBEXECDIR}")
endif()

