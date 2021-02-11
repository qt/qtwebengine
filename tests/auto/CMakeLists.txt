if(QT_FEATURE_qtwebengine_build
    OR (QT_BUILD_STANDALONE_TESTS AND TARGET Qt::WebEngineCore))
    add_subdirectory(core)
    add_subdirectory(httpserver)
    add_subdirectory(util)
endif()
if(QT_FEATURE_qtwebengine_quick_build
    OR (QT_BUILD_STANDALONE_TESTS AND TARGET Qt::WebEngineQuick))
    add_subdirectory(quick)
endif()
if(QT_FEATURE_qtwebengine_widgets_build
    OR (QT_BUILD_STANDALONE_TESTS AND TARGET Qt::WebEngineWidgets))
    add_subdirectory(widgets)
endif()
#if(QT_FEATURE_build_qtpdf AND QT_FEATURE_webengine_qtpdf_support)
#    add_subdirectory(pdf)
#endif()
