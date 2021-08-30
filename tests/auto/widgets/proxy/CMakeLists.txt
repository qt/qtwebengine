include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_webengine_proxy
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::WebEngineWidgets
       Test::HttpServer
)
