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

install(TARGETS ${qtWebEngineProcessName} RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}")

make_install_only(${qtWebEngineProcessName})
