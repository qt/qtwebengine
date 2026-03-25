# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core Gui QuickControls2 WebEngineQuick)

qt_standard_project_setup(REQUIRES 6.8)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/lifecycle")

qt_add_executable(lifecycle
    main.cpp
)

set_target_properties(lifecycle PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(lifecycle PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineQuick
)

qt_add_qml_module(lifecycle
    URI LifecycleUtils
    VERSION 1.0
    QML_FILES
        "WebBrowser.qml"
        "WebTab.qml"
        "WebTabBar.qml"
        "WebTabButton.qml"
        "WebTabStack.qml"
        "WebToolButton.qml"
    SOURCES
        utils.h
)

set(resources_resource_files
    "qtquickcontrols2.conf"
)

qt_add_resources(lifecycle "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

install(TARGETS lifecycle
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
