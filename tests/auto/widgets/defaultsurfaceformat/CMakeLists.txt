include(../../util/util.cmake)

qt_internal_add_test(tst_defaultsurfaceformat
    SOURCES
        tst_defaultsurfaceformat.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_defaultsurfaceformat_resource_files
    "resources/index.html"
)

qt_internal_add_resource(tst_defaultsurfaceformat "tst_defaultsurfaceformat"
    PREFIX
        "/"
    FILES
        ${tst_defaultsurfaceformat_resource_files}
)
