if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
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
   target_link_libraries(${qtWebEngineProcessName} PRIVATE WebEngineCoreSandbox)
   set_property(TARGET ${qtWebEngineProcessName} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(${qtWebEngineProcessName}
   PUBLIC
       Qt::Gui
       Qt::WebEngineCore
   PRIVATE
       Qt::CorePrivate
       Qt::WebEngineCorePrivate
)

target_include_directories(${qtWebEngineProcessName} PRIVATE ../core)

qt_get_cmake_configurations(configs)
foreach(config ${configs})
    string(TOUPPER "${config}" config_upper)
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}")
endforeach()

if(QT_FEATURE_debug_and_release)
    set_target_properties(${qtWebEngineProcessName} PROPERTIES
                          OUTPUT_NAME_DEBUG ${qtWebEngineProcessName}${CMAKE_DEBUG_POSTFIX})
endif()

if(isFramework)
    set_target_properties(${qtWebEngineProcessName} PROPERTIES MACOSX_BUNDLE TRUE)
    target_sources(${qtWebEngineProcessName} PRIVATE QtWebEngineProcess.entitlements)
    set_source_files_properties(QtWebEngineProcess.entitlements PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    add_custom_command(TARGET ${qtWebEngineProcessName} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
              "$<TARGET_BUNDLE_DIR:WebEngineCore>/Versions/A/Helpers/${qtWebEngineProcessName}.app"
          COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Helpers"
              "$<TARGET_BUNDLE_DIR:WebEngineCore>/Helpers"
    )
    install(TARGETS ${qtWebEngineProcessName}
        BUNDLE DESTINATION ./lib/QtWebEngineCore.framework/Versions/A/Helpers
        COMPONENT Runtime
    )
else()
    install(TARGETS ${qtWebEngineProcessName}
        CONFIGURATIONS ${configs}
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
    )
endif()

if(MSVC)
    install(FILES "$<TARGET_PDB_FILE:${qtWebEngineProcessName}>" OPTIONAL
        DESTINATION "${INSTALL_LIBEXECDIR}"
    )
endif()

make_install_only(${qtWebEngineProcessName})
