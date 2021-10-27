cmake_minimum_required(VERSION 3.16)
project(videoplayer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/videoplayer")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WebEngineWidgets)

qt_add_executable(videoplayer
    fullscreennotification.cpp fullscreennotification.h
    fullscreenwindow.cpp fullscreenwindow.h
    main.cpp
    mainwindow.cpp mainwindow.h
)
set_target_properties(videoplayer PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(videoplayer PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)


# Resources:
set(videoplayer_resource_files
    "data/index.html"
)

qt_add_resources(videoplayer "videoplayer"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${videoplayer_resource_files}
)

install(TARGETS videoplayer
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
