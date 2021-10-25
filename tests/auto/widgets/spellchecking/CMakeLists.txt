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
    ABSOLUTE ${CMAKE_CURRENT_LIST_DIR}/dict
    *.dic
)

foreach(dictFile ${dicts})
    qt_add_webengine_dictionary(
        TARGET tst_spellchecking
        SOURCE "${dictFile}"
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
