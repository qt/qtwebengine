if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." REALPATH)
endif()
find_package(Qt6 ${PROJECT_VERSION} REQUIRED COMPONENTS Gui Network OpenGL Quick)
find_package(Qt6 ${PROJECT_VERSION} QUIET OPTIONAL_COMPONENTS PrintSupport WebChannel Positioning)
include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

get_configure_mode(configureMode)

set(qtWebEngineProcessName QtWebEngineProcess)

qt_internal_add_module(WebEngineCore
     ${configureMode}
     SOURCES
        qtwebenginecoreglobal.cpp qtwebenginecoreglobal.h qtwebenginecoreglobal_p.h
        qwebenginecallback.h qwebenginecallback_p.h
        qwebenginecertificateerror.cpp qwebenginecertificateerror.h
        qwebengineclientcertificateselection.cpp qwebengineclientcertificateselection.h
        qwebengineclientcertificatestore.cpp qwebengineclientcertificatestore.h
        qwebenginecontextmenurequest.cpp qwebenginecontextmenurequest.h qwebenginecontextmenurequest_p.h
        qwebenginecookiestore.cpp qwebenginecookiestore.h qwebenginecookiestore_p.h
        qwebenginedownloadrequest.cpp qwebenginedownloadrequest.h qwebenginedownloadrequest_p.h
        qwebenginefindtextresult.cpp qwebenginefindtextresult.h
        qwebenginefullscreenrequest.cpp qwebenginefullscreenrequest.h
        qwebenginehistory.cpp qwebenginehistory.h qwebenginehistory_p.h
        qwebenginehttprequest.cpp qwebenginehttprequest.h
        qwebengineloadrequest.cpp qwebengineloadrequest.h
        qwebenginemessagepumpscheduler.cpp qwebenginemessagepumpscheduler_p.h
        qwebenginenotification.cpp qwebenginenotification.h
        qwebenginepage.cpp qwebenginepage.h qwebenginepage_p.h
        qwebengineprofile.cpp qwebengineprofile.h qwebengineprofile_p.h
        qwebenginequotarequest.cpp qwebenginequotarequest.h
        qwebengineregisterprotocolhandlerrequest.cpp qwebengineregisterprotocolhandlerrequest.h
        qwebenginescript.cpp qwebenginescript.h
        qwebenginescriptcollection.cpp qwebenginescriptcollection.h qwebenginescriptcollection_p.h
        qwebenginesettings.cpp qwebenginesettings.h
        qwebengineurlrequestinfo.cpp qwebengineurlrequestinfo.h qwebengineurlrequestinfo_p.h
        qwebengineurlrequestinterceptor.h
        qwebengineurlrequestjob.cpp qwebengineurlrequestjob.h
        qwebengineurlscheme.cpp qwebengineurlscheme.h
        qwebengineurlschemehandler.cpp qwebengineurlschemehandler.h
    DEFINES
        BUILDING_CHROMIUM
        NOMINMAX
    INCLUDE_DIRECTORIES
        ../
        ../../3rdparty/chromium
        ../../3rdparty/chromium/third_party/abseil-cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Quick
)

set_target_properties(WebEngineCore PROPERTIES QTWEBENGINEPROCESS_NAME ${qtWebEngineProcessName})

# Chromium included headers are not clean
qt_skip_warnings_are_errors(WebEngineCore)
make_install_only(WebEngineCore)
make_config_for_gn(WebEngineCore gn_config)

qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_printing_and_pdf
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)
qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_webchannel
    PUBLIC_LIBRARIES
        Qt::WebChannel
)
qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_geolocation
    PUBLIC_LIBRARIES
        Qt::Positioning
)
