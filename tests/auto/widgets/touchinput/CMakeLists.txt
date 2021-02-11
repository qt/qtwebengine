include(../../util/util.cmake)

qt_internal_add_test(tst_touchinput
    SOURCES
        tst_touchinput.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Qt::GuiPrivate
        Test::Util
)
