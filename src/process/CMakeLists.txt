if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
endif()
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

find_package(Qt6 COMPONENTS Gui)

get_target_property(qtWebEngineProcessName WebEngineCore QTWEBENGINEPROCESS_NAME)
get_target_property(isFramework WebEngineCore FRAMEWORK)

qt_internal_add_executable(${qtWebEngineProcessName} SOURCES main.cpp)

if(WIN32)
   target_sources(${qtWebEngineProcessName} PRIVATE support_win.cpp)
   target_link_libraries(${qtWebEngineProcessName} PRIVATE WebEngineCoreSandbox)
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
get_install_config(config)

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
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
        CONFIGURATIONS ${config}
    )
endif()

make_install_only(${qtWebEngineProcessName})
