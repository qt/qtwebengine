cmake_minimum_required(VERSION 3.18.0)

project(QtWebEngineCore
    VERSION "6.0.0"
    DESCRIPTION "QtWebEngineCore"
    HOMEPAGE_URL "https://doc.qt.io/qt-5/qtwebenginecore-module.html"
    LANGUAGES CXX C)

if (NOT DEFINED CMAKE_WEBENGINE_ROOT_BUILD_PATH)
    get_filename_component(CMAKE_WEBENGINE_ROOT_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. ABSOLUTE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../../cmake")
set(buildDir "${CMAKE_CURRENT_BINARY_DIR}/debug")

include(${PROJECT_SOURCE_DIR}/../../cmake/Functions.cmake)

get_qt_features(featureList webengine)
message(${featureList})

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui Widgets Network OpenGL OpenGLWidgets Quick Qml QuickTest Test)
find_package(Gn REQUIRED)
find_package(Ninja REQUIRED)
find_package(Python2 REQUIRED)

qt_build_repo_begin()

include(generator.cmake)
configure_file(${PROJECT_SOURCE_DIR}/configure/BUILD.toolchain.gn.in ${buildDir}/toolchain/BUILD.gn @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/configure/BUILD.root.gn.in ${buildDir}/BUILD.gn @ONLY)

get_target_property(gnCmd Gn::gn IMPORTED_LOCATION)

set(gnArg help)

message("configure with gn")
execute_process(
    COMMAND ${gnCmd} ${gnArg}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE gnResult
    OUTPUT_VARIABLE gnOutput
    ERROR_VARIABLE gnError
    )

if(NOT gnResult EQUAL 0)
    message(FATAL_ERROR "Calling gn - FAILED ! \n${gnOutput}\n${gnError}")
endif()

add_custom_command(
    OUTPUT QtWebEngineCore.stamp
    WORKING_DIRECTORY ${buildDir}
    COMMAND Ninja::ninja
            $ENV{NINJAFLAGS}
            -C ${buildDir}
            ${CMAKE_PROJECT_NAME}
    USES_TERMINAL
    VERBATIM
)
add_custom_target(ninja ALL DEPENDS QtWebEngineCore.stamp)

message("CMD: ${gn_cmd}\nARGS: ${gnArg}\nOUTPUT: ${gnOutput}")

qt_build_repo_end()

