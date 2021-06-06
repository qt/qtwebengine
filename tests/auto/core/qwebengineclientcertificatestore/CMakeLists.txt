qt_internal_add_test(tst_qwebengineclientcertificatestore
    SOURCES
        tst_qwebengineclientcertificatestore.cpp
    LIBRARIES
        Qt::WebEngineCore
)

set(tst_qwebengineclientcertificatestore_resource_files
    "resources/certificate.crt"
    "resources/certificate1.crt"
    "resources/privatekey.key"
    "resources/privatekey1.key"
)

qt_internal_add_resource(tst_qwebengineclientcertificatestore "tst_qwebengineclientcertificatestore"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineclientcertificatestore_resource_files}
)

