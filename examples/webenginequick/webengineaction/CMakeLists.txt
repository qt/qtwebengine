cmake_minimum_required(VERSION 3.16)
project(webengineaction LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/webengineaction")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)

qt_add_executable(webengineaction
    main.cpp
)

set_target_properties(webengineaction PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(webengineaction PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_add_resources(webengineaction "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS webengineaction
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
