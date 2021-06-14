include(../../util/util.cmake)
include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_qwebengineurlrequestinterceptor
    SOURCES
        tst_qwebengineurlrequestinterceptor.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
        Test::Util
)

set(tst_qwebengineurlrequestinterceptor_resource_files
    "resources/content.html"
    "resources/favicon.html"
    "resources/firstparty.html"
    "resources/fontawesome.woff"
    "resources/icons/favicon.png"
    "resources/iframe.html"
    "resources/iframe2.html"
    "resources/iframe3.html"
    "resources/image.html"
    "resources/image_in_iframe.html"
    "resources/index.html"
    "resources/media.html"
    "resources/media.mp4"
    "resources/media_in_iframe.html"
    "resources/resource.html"
    "resources/resource_in_iframe.html"
    "resources/script.js"
    "resources/style.css"
    "resources/sw.html"
    "resources/sw.js"
)

qt_internal_add_resource(tst_qwebengineurlrequestinterceptor "tst_qwebengineurlrequestinterceptor"
    PREFIX
        "/"
    FILES
        ${tst_qwebengineurlrequestinterceptor_resource_files}
)
