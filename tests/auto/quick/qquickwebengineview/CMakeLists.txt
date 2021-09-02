include(../../util/util.cmake)

qt_internal_add_test(tst_qquickwebengineview
    SOURCES
        tst_qquickwebengineview.cpp
    LIBRARIES
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuick
        Qt::GuiPrivate
        Qt::WebEngineQuickPrivate
        Qt::TestPrivate
        Test::Util
)
