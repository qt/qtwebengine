if(NOT QT_BUILD_STANDALONE_TESTS)
    cmake_minimum_required(VERSION 3.19)

    include(${CMAKE_CURRENT_LIST_DIR}/../.cmake.conf)

    project(QtWebEngineTests
        VERSION "${QT_REPO_MODULE_VERSION}"
        DESCRIPTION "Qt WebEngine Tests"
        HOMEPAGE_URL "https://qt.io/"
        LANGUAGES CXX C
    )

    # Make sure we use the fixed BASE argument of qt_add_resource.
    set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui Widgets
    WebEngineCore WebEngineWidgets WebEngineQuick QuickWidgets Test QuickTest)

if(NOT QT_BUILD_STANDALONE_TESTS)
    qt_build_repo_begin()
    qt_build_tests()
    qt_build_repo_end()
else()
    qt_build_tests()
endif()
