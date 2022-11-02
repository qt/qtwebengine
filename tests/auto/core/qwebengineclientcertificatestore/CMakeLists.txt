include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineclientcertificatestore
    SOURCES
        tst_qwebengineclientcertificatestore.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Test::Util
)

set(tst_qwebengineclientcertificatestore_resource_files
    "resources/certificate.crt"
    "resources/certificate1.crt"
    "resources/privatekey.key"
    "resources/privatekey1.key"
    "resources/server.pem"
    "resources/server.key"
    "resources/client.pem"
    "resources/client.key"
    "resources/ca.pem"
)

qt_internal_add_resource(tst_qwebengineclientcertificatestore "tst_qwebengineclientcertificatestore"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineclientcertificatestore_resource_files}
)

