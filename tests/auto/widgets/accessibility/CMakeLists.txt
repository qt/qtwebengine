include(../../util/util.cmake)

qt_internal_add_test(tst_accessibility
    SOURCES
        tst_accessibility.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)
