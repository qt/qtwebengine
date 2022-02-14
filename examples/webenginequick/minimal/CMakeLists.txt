cmake_minimum_required(VERSION 3.16)
project(minimal LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/minimal-qml")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)

qt_add_executable(webengine-minimal-qml
    main.cpp
)

set_target_properties(webengine-minimal-qml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(webengine-minimal-qml PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_add_resources(webengine-minimal-qml "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS webengine-minimal-qml
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
