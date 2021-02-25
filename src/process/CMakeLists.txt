if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
endif()
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

find_package(Qt6 COMPONENTS Gui)

get_target_property(qtWebEngineProcessName WebEngineCore QTWEBENGINEPROCESS_NAME)

qt_internal_add_executable(${qtWebEngineProcessName} SOURCES main.cpp)

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

install(TARGETS ${qtWebEngineProcessName}
     RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
     CONFIGURATIONS ${config}
)

make_install_only(${qtWebEngineProcessName})
