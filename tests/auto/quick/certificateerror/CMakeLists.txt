include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_certificateerror_quick
    SOURCES
        testhandler.cpp testhandler.h
        tst_certificateerror.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::WebEngineQuickPrivate
        Test::HttpServer
        Test::Util
)

set(certificateerror_resource_files
    "resources/server.pem"
    "resources/server.key"
    "WebView.qml"
)

qt_add_resource(tst_certificateerror_quick "certificateerror"
    PREFIX
        "/"
    FILES
        ${certificateerror_resource_files}
)
