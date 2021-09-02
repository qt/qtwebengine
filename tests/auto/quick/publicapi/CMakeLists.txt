qt_internal_add_test(tst_publicapi
    SOURCES
        tst_publicapi.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
)
