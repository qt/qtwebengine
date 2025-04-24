# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(simplebrowser LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_standard_project_setup()

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
    webauthdialog.cpp webauthdialog.h webauthdialog.ui
)

if(WIN32)
    set_property(
        TARGET simplebrowser
        APPEND PROPERTY
            SOURCES simplebrowser.exe.manifest)
endif()

set_target_properties(simplebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.webenginewidgets.simplebrowser"
)

target_link_libraries(simplebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

# Resources:
set(simplebrowser_resource_files
    "data/AppLogoColor.png"
    "data/ninja.png"
)

qt_add_resources(simplebrowser "simplebrowser"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${simplebrowser_resource_files}
)

set(simplebrowser1_resource_files
    "data/3rdparty/dialog-error.png"
    "data/3rdparty/edit-clear.png"
    "data/3rdparty/go-bottom.png"
    "data/3rdparty/go-next.png"
    "data/3rdparty/go-previous.png"
    "data/3rdparty/process-stop.png"
    "data/3rdparty/text-html.png"
    "data/3rdparty/view-refresh.png"
)

qt_add_resources(simplebrowser "simplebrowser1"
    PREFIX
        "/"
    BASE
        "data/3rdparty"
    FILES
        ${simplebrowser1_resource_files}
)

if (APPLE)
    set_target_properties(simplebrowser PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.cmake.macos.plist"
    )

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        # Need to sign application for location permissions to work
        if(QT_FEATURE_debug_and_release)
            set(exe_path "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/")
        else()
            unset(exe_path)
        endif()
        add_custom_command(TARGET simplebrowser
            POST_BUILD COMMAND codesign --force -s - ${exe_path}simplebrowser.app
        )
    endif()
endif()

install(TARGETS simplebrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET simplebrowser
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
