qt_internal_add_test(tst_offscreen
    SOURCES
        tst_offscreen.cpp
    LIBRARIES
        Qt::WebEngineWidgets
)

set(tst_offscreen_resource_files
    "test.html"
)

set_tests_properties(tst_offscreen PROPERTIES
    ENVIRONMENT QT_QPA_PLATFORM=offscreen
)

qt_internal_add_resource(tst_offscreen "tst_offscreen"
    PREFIX
        "/"
    FILES
        ${tst_offscreen_resource_files}
)

