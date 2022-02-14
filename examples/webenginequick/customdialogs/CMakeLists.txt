cmake_minimum_required(VERSION 3.16)
project(customdialogs LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/customdialogs")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineQuick)

qt_add_executable(customdialogs
    main.cpp
    server.cpp server.h
)

set_target_properties(customdialogs PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(customdialogs PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

set(customdialogs_resource_files
    "MessageRectangle.qml"
    "SwitchButton.qml"
    "WebView.qml"
    "forms/Authentication.qml"
    "forms/AuthenticationForm.ui.qml"
    "forms/ColorCell.qml"
    "forms/ColorPicker.qml"
    "forms/ColorPickerForm.ui.qml"
    "forms/CustomButton.qml"
    "forms/FilePicker.qml"
    "forms/FilePickerForm.ui.qml"
    "forms/FileRow.qml"
    "forms/JavaScript.qml"
    "forms/JavaScriptForm.ui.qml"
    "forms/Menu.qml"
    "forms/MenuForm.ui.qml"
    "forms/TouchSelectionMenu.qml"
    "forms/TouchSelectionMenuForm.ui.qml"
    "icon.svg"
    "index.html"
    "main.qml"
    "style.css"
)

qt_add_resources(customdialogs "customdialogs"
    PREFIX
        "/"
    FILES
        ${customdialogs_resource_files}
)

if(TARGET Qt::Widgets)
    target_link_libraries(customdialogs PUBLIC
        Qt::Widgets
    )
endif()

install(TARGETS customdialogs
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
