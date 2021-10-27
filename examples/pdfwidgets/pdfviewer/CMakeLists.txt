# Generated from pdfviewer.pro.

cmake_minimum_required(VERSION 3.16)
project(pdfviewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/pdfwidgets/pdfviewer")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(pdfviewerwidgets
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    pageselector.cpp pageselector.h
    zoomselector.cpp zoomselector.h
)
set_target_properties(pdfviewerwidgets PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(pdfviewerwidgets PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::PdfWidgets
)


# Resources:
set(resources_resource_files
    "images/busy.png"
    "images/fileopen.png"
    "images/go-next-24.png"
    "images/go-previous-24.png"
    "images/zoom-in-24.png"
    "images/zoom-in-32.png"
    "images/zoom-out-24.png"
    "images/zoom-out-32.png"
)

qt_add_resources(pdfviewerwidgets "resources"
    PREFIX
        "/icons"
    FILES
        ${resources_resource_files}
)

install(TARGETS pdfviewerwidgets
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
