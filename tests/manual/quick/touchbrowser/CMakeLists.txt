cmake_minimum_required(VERSION 3.16)
project(touchbrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_MANUAL_TESTSDIR)
    set(INSTALL_MANUAL_TESTSDIR "manual")
endif()

set(INSTALL_MANUAL_TESTDIR "${INSTALL_MANUAL_TESTSDIR}/quick/touchbrowser")

if(NOT CMAKE_CROSSCOMPILING)
    add_definitions(-DDESKTOP_BUILD)
    set(SOURCES "touchmockingapplication.cpp")
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick WebEngineQuick Gui)

qt_add_executable(touchbrowser
    main.cpp
    utils.h
    ${SOURCES}
)

set_target_properties(touchbrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(touchbrowser PRIVATE
    Qt::GuiPrivate
    Qt::Quick
    Qt::WebEngineQuick
)

set(touchbrowser_resource_files
    "AddressBar.qml"
    "main.qml"
    "MockTouchPoint.qml"
    "touchpoint.png"
)

qt_add_resources(touchbrowser "touchbrowser"
    PREFIX
    "/"
    FILES
    ${touchbrowser_resource_files}
)

install(TARGETS touchbrowser
    RUNTIME DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    BUNDLE DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    LIBRARY DESTINATION "${INSTALL_MANUAL_TESTDIR}"
)
