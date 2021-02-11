include(../../util/util.cmake)

qt_internal_add_test(tst_faviconmanager
    SOURCES
        tst_faviconmanager.cpp
    LIBRARIES
        Qt::WebEngineWidgets
        Test::Util
)

set(tst_faviconmanager_resource_files
    "resources/favicon-misc.html"
    "resources/favicon-multi.html"
    "resources/favicon-shortcut.html"
    "resources/favicon-single.html"
    "resources/favicon-touch.html"
    "resources/favicon-unavailable.html"
    "resources/icons/qt144.png"
    "resources/icons/qt32.ico"
    "resources/icons/qtmulti.ico"
    "resources/test1.html"
)

qt_add_resource(tst_faviconmanager "tst_faviconmanager"
    PREFIX
        "/"
    FILES
        ${tst_faviconmanager_resource_files}
)
