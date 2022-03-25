cmake_minimum_required(VERSION 3.16)
project(webgl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_MANUAL_TESTSDIR)
    set(INSTALL_MANUAL_TESTSDIR "manual")
endif()

set(INSTALL_MANUAL_TESTDIR "${INSTALL_MANUAL_TESTSDIR}/widgets/webgl")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(webgl
    main.cpp
)

set_target_properties(webgl PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(webgl PRIVATE
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

install(TARGETS webgl
    RUNTIME DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    BUNDLE DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    LIBRARY DESTINATION "${INSTALL_MANUAL_TESTDIR}"
)
