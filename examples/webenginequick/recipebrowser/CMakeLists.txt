cmake_minimum_required(VERSION 3.16)
project(recipebrowser LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginequick/recipebrowser")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 WebEngineQuick)

qt_add_executable(recipebrowser
    main.cpp
)

set_target_properties(recipebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(recipebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::WebEngineQuick
)

# Resources:
set_source_files_properties("resources/pages/assets/3rdparty/markdown.css"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/assets/3rdparty/marked.js"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/assets/custom.css"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/assets/custom.js"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/burger.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/cupcakes.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/burger.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/cupcakes.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/pasta.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/pizza.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/skewers.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/soup.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/images/steak.jpg"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/pasta.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/pizza.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/skewers.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/soup.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/pages/steak.html"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/qml/RecipeList.qml"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set_source_files_properties("resources/qml/main.qml"
    PROPERTIES QT_SKIP_QUICKCOMPILER 1
)

set(resources_resource_files
    "resources/pages/assets/3rdparty/markdown.css"
    "resources/pages/assets/3rdparty/marked.js"
    "resources/pages/assets/custom.css"
    "resources/pages/assets/custom.js"
    "resources/pages/burger.html"
    "resources/pages/cupcakes.html"
    "resources/pages/images/burger.jpg"
    "resources/pages/images/cupcakes.jpg"
    "resources/pages/images/pasta.jpg"
    "resources/pages/images/pizza.jpg"
    "resources/pages/images/skewers.jpg"
    "resources/pages/images/soup.jpg"
    "resources/pages/images/steak.jpg"
    "resources/pages/pasta.html"
    "resources/pages/pizza.html"
    "resources/pages/skewers.html"
    "resources/pages/soup.html"
    "resources/pages/steak.html"
    "resources/qml/main.qml"
    "resources/qml/RecipeList.qml"
)

qt_add_resources(recipebrowser "resources"
    PREFIX
        "/"
    BASE
        "resources"
    FILES
        ${resources_resource_files}
)

if(CMAKE_CROSSCOMPILING AND (LINUX OR QNX OR posix))
    target_compile_definitions(recipebrowser PUBLIC
        QTWEBENGINE_RECIPE_BROWSER_EMBEDDED
    )
endif()

install(TARGETS recipebrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
