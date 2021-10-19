cmake_minimum_required(VERSION 3.16)
project(spellchecker LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/spellchecker")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WebEngineWidgets)

qt_add_executable(spellchecker
    main.cpp
    webview.cpp webview.h
)
set_target_properties(spellchecker PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(spellchecker PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)


# Resources:
set(spellchecker_resource_files
    "data/icon.svg"
    "data/index.html"
    "data/style.css"
)

qt6_add_resources(spellchecker "spellchecker"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${spellchecker_resource_files}
)

install(TARGETS spellchecker
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

file(GLOB_RECURSE dicts
    RELATIVE ${CMAKE_CURRENT_LIST_DIR}/dict
    */*.dic
)

if(QT_GENERATOR_IS_MULTI_CONFIG)
    set(spellcheckerDir ${CMAKE_CURRENT_BINARY_DIR}/dict/qtwebengine_dictionaries)
else()
    set(spellcheckerDir ${CMAKE_CURRENT_BINARY_DIR}/qtwebengine_dictionaries)
endif()

# note setting QT_WEBENGINE_ICU_DATA below is not required if this example
# is not compiled as part of qtwebengine module build

foreach(dictFile ${dicts})
    get_filename_component(dictName ${dictFile} NAME_WE)
    add_custom_command(TARGET spellchecker
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${spellcheckerDir}
        COMMAND ${CMAKE_COMMAND} -E env
        $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qwebengine_convert_dict>
        ${CMAKE_CURRENT_SOURCE_DIR}/dict/${dictFile} ${spellcheckerDir}/${dictName}.bdic
        COMMENT "Running qwebengine_convert_dict"
    )
endforeach()

# copy dictionaries to $<CONFIG> build dir
if(QT_GENERATOR_IS_MULTI_CONFIG)
    add_custom_command(TARGET spellchecker
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E echo Copying dictionares
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/dict
       ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    )
endif()

