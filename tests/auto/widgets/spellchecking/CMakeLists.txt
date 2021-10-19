include(../../util/util.cmake)

qt_internal_add_test(tst_spellchecking
    SOURCES
        tst_spellchecking.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

qt_internal_add_resource(tst_spellchecking "tst_spellchecking"
    PREFIX
        "/"
    FILES
        "resources/index.html"
)

file(GLOB_RECURSE dicts
    RELATIVE ${CMAKE_CURRENT_LIST_DIR}/dict
    *.dic
)

if(QT_GENERATOR_IS_MULTI_CONFIG)
    set(spellcheckerDir ${CMAKE_CURRENT_BINARY_DIR}/dict/qtwebengine_dictionaries)
else()
    set(spellcheckerDir ${CMAKE_CURRENT_BINARY_DIR}/qtwebengine_dictionaries)
endif()

foreach(dictFile ${dicts})
    get_filename_component(dictName ${dictFile} NAME_WE)
    add_custom_command(TARGET tst_spellchecking
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${spellcheckerDir}
        COMMAND ${CMAKE_COMMAND} -E env
        $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qwebengine_convert_dict>
        ${CMAKE_CURRENT_SOURCE_DIR}/dict/${dictFile}
        ${spellcheckerDir}/${dictName}.bdic
        COMMENT "Running qwebengine_convert_dict"
    )
endforeach()

# copy dictionaries to $<CONFIG> build dir
if(QT_GENERATOR_IS_MULTI_CONFIG)
    add_custom_command(TARGET tst_spellchecking
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E echo Copying dictionares
       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/dict
       ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    )
endif()
