cmake_minimum_required(VERSION 3.16)
project(inputmethods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_MANUAL_TESTSDIR)
    set(INSTALL_MANUAL_TESTSDIR "manual")
endif()

set(INSTALL_MANUAL_TESTDIR "${INSTALL_MANUAL_TESTSDIR}/widgets/inputmethods")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets Test)

qt_add_executable(inputmethods
    colorpicker.cpp
    controlview.cpp
    main.cpp
    referenceview.cpp
    testview.cpp
    webview.cpp
)

set_target_properties(inputmethods PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(inputmethods PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    Qt::WebEngineWidgets
)

set(inputmethods_resource_files
    "testdata.csv"
)

qt_add_resources(inputmethods "inputmethods"
    PREFIX
    "/"
    FILES
    ${inputmethods_resource_files}
)

install(TARGETS inputmethods
    RUNTIME DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    BUNDLE DESTINATION "${INSTALL_MANUAL_TESTDIR}"
    LIBRARY DESTINATION "${INSTALL_MANUAL_TESTDIR}"
)
