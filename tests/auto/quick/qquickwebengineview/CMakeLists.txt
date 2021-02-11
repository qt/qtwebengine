include(../../util/util.cmake)

qt_internal_add_test(tst_qquickwebengineview
    SOURCES
        tst_qquickwebengineview.cpp
    LIBRARIES
        Qt::WebEngineCorePrivate
        Qt::WebEngineWidgets
        Qt::WebEngineQuick
        Qt::GuiPrivate
        Qt::WebEngineQuickPrivate
        Test::Util
)
