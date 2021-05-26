cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/../.cmake.conf)

project(QtWebEngineExamples
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt WebEngine Examples"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS WebEngineCore WebEngineWidgets WebEngineQuick)
qt_build_repo_begin()
qt_examples_build_begin()

if(QT_FEATURE_qtwebengine_build AND QT_FEATURE_qtwebengine_quick_build)
    add_subdirectory(webengine)
endif()
if(QT_FEATURE_qtwebengine_build AND QT_FEATURE_qtwebengine_widgets_build)
    add_subdirectory(webenginewidgets)
endif()
if(QT_FEATURE_qtpdf_build )
    add_subdirectory(pdf)
endif()
if(QT_FEATURE_qtpdf_build AND QT_FEATURE_pdf_widgets)
    add_subdirectory(pdfwidgets)
endif()

qt_examples_build_end()
qt_build_repo_end()
