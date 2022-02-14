cmake_minimum_required(VERSION 3.16)
project(stylesheetbrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/stylesheetbrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(stylesheetbrowser
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    stylesheetdialog.cpp stylesheetdialog.h stylesheetdialog.ui
)

set_target_properties(stylesheetbrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(stylesheetbrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(stylesheetbrowser_resource_files
    "3rdparty/view-refresh.png"
)

qt_add_resources(stylesheetbrowser "stylesheetbrowser"
    PREFIX
        "/"
    BASE
        "3rdparty"
    FILES
        ${stylesheetbrowser_resource_files}
)

install(TARGETS stylesheetbrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
