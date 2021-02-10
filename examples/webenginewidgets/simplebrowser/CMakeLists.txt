cmake_minimum_required(VERSION 3.18)
project(simplebrowser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/simplebrowser")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WebEngineWidgets)

qt_add_executable(simplebrowser
    browser.cpp browser.h
    browserwindow.cpp browserwindow.h
    certificateerrordialog.ui
    downloadmanagerwidget.cpp downloadmanagerwidget.h downloadmanagerwidget.ui
    downloadwidget.cpp downloadwidget.h downloadwidget.ui
    main.cpp
    passworddialog.ui
    tabwidget.cpp tabwidget.h
    webpage.cpp webpage.h
    webpopupwindow.cpp webpopupwindow.h
    webview.cpp webview.h
)
set_target_properties(simplebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(simplebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)


# Resources:
set(simplebrowser_resource_files
    "AppLogoColor.png"
    "ninja.png"
)

qt6_add_resources(simplebrowser "simplebrowser"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${simplebrowser_resource_files}
)
set_source_files_properties("data/3rdparty/dialog-error.png"
    PROPERTIES QT_RESOURCE_ALIAS "dialog-error.png"
)
set_source_files_properties("data/3rdparty/edit-clear.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit-clear.png"
)
set_source_files_properties("data/3rdparty/go-bottom.png"
    PROPERTIES QT_RESOURCE_ALIAS "go-bottom.png"
)
set_source_files_properties("data/3rdparty/go-next.png"
    PROPERTIES QT_RESOURCE_ALIAS "go-next.png"
)
set_source_files_properties("data/3rdparty/go-previous.png"
    PROPERTIES QT_RESOURCE_ALIAS "go-previous.png"
)
set_source_files_properties("data/3rdparty/process-stop.png"
    PROPERTIES QT_RESOURCE_ALIAS "process-stop.png"
)
set_source_files_properties("data/3rdparty/text-html.png"
    PROPERTIES QT_RESOURCE_ALIAS "text-html.png"
)
set_source_files_properties("data/3rdparty/view-refresh.png"
    PROPERTIES QT_RESOURCE_ALIAS "view-refresh.png"
)
set(simplebrowser1_resource_files
    "3rdparty/dialog-error.png"
    "3rdparty/edit-clear.png"
    "3rdparty/go-bottom.png"
    "3rdparty/go-next.png"
    "3rdparty/go-previous.png"
    "3rdparty/process-stop.png"
    "3rdparty/text-html.png"
    "3rdparty/view-refresh.png"
)

qt6_add_resources(simplebrowser "simplebrowser1"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${simplebrowser1_resource_files}
)

install(TARGETS simplebrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
