include(../../util/util.cmake)

qt_internal_add_test(tst_qquickwebenginedefaultsurfaceformat
    SOURCES
        tst_qquickwebenginedefaultsurfaceformat.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
        Test::Util
)
