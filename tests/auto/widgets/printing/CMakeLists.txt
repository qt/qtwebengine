include(../../util/util.cmake)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(POPPLER_CPP poppler-cpp IMPORTED_TARGET)
endif()

qt_internal_add_test(tst_printing
    SOURCES
        tst_printing.cpp
    PUBLIC_LIBRARIES
        Qt::WebEngineWidgets
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineCorePrivate
        Test::Util
)

qt_internal_extend_target(tst_printing
    CONDITION POPPLER_CPP_FOUND AND QT_FEATURE_webengine_system_poppler
    LIBRARIES
        PkgConfig::POPPLER_CPP
)

set(tst_printing_resource_files
    "resources/basic_printing_page.html"
)

qt_internal_add_resource(tst_printing "tst_printing"
    PREFIX "/"
    FILES ${tst_printing_resource_files}
)
