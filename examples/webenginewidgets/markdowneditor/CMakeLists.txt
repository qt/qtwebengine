cmake_minimum_required(VERSION 3.16)
project(markdowneditor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/markdowneditor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebChannel WebEngineWidgets)

qt_add_executable(markdowneditor
    document.cpp document.h
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    previewpage.cpp previewpage.h
)

set_target_properties(markdowneditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(markdowneditor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebChannel
    Qt::WebEngineWidgets
)

# Resources:
set(markdowneditor_resource_files
    "resources/3rdparty/markdown.css"
    "resources/3rdparty/marked.js"
    "resources/default.md"
    "resources/index.html"
)

qt_add_resources(markdowneditor "markdowneditor"
    PREFIX
        "/"
    BASE
        "resources"
    FILES
        ${markdowneditor_resource_files}
)

install(TARGETS markdowneditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
