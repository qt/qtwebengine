include(../../httpserver/httpserver.cmake)

qt_internal_add_test(tst_proxypac_file
    SOURCES
        tst_proxypac.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
)

if(WIN32)
    get_filename_component(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
    set(fileEnvArg "--proxy-pac-url=\"file:///${SOURCE_DIR}/proxy.pac\"")
elseif(boot2qt)
    set(fileEnvArg "--single-process --no-sandbox --proxy-pac-url=\"file://${CMAKE_CURRENT_LIST_DIR}/proxy.pac\"")
else()
    set(fileEnvArg "--proxy-pac-url=\"file://${CMAKE_CURRENT_LIST_DIR}/proxy.pac\"")
endif()

set_tests_properties(tst_proxypac_file PROPERTIES
    ENVIRONMENT QTWEBENGINE_CHROMIUM_FLAGS=${fileEnvArg}
)

qt_internal_add_test(tst_proxypac_qrc
    SOURCES
        tst_proxypac.cpp
    LIBRARIES
        Qt::WebEngineCore
        Test::HttpServer
)

if(boot2qt)
    set(qrcEnvArg "--single-process --no-sandbox --proxy-pac-url=\"qrc:///proxy.pac\"")
else()
    set(qrcEnvArg "--proxy-pac-url=\"qrc:///proxy.pac\"")
endif()

set_tests_properties(tst_proxypac_qrc PROPERTIES
    ENVIRONMENT QTWEBENGINE_CHROMIUM_FLAGS=${qrcEnvArg}
)

qt_internal_add_resource(tst_proxypac_qrc "proxypac"
    PREFIX
        "/"
    FILES
        "proxy.pac"
)
