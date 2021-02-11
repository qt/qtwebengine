include(../../util/util.cmake)

qt_internal_add_test(tst_qwebenginesettings
    SOURCES
        tst_qwebenginesettings.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)
