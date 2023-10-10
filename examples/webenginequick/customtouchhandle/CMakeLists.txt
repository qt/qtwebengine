cmake_minimum_required(VERSION 3.16)
project(customtouchhandle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/customtouchhandle")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)

qt_add_executable(customtouchhandle
    main.cpp
)

set_target_properties(customtouchhandle PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(customtouchhandle PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(customtouchhandle "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS customtouchhandle
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
