find_package(Qt6 ${PROJECT_VERSION} REQUIRED COMPONENTS Gui Network Quick)
find_package(Qt6 ${PROJECT_VERSION} QUIET OPTIONAL_COMPONENTS WebChannel Positioning)

set(qtWebEngineProcessName QtWebEngineProcess)

qt_internal_add_module(WebEngineCore
     SOURCES
        qtwebenginecoreglobal.cpp qtwebenginecoreglobal.h qtwebenginecoreglobal_p.h
        qwebenginecertificateerror.cpp qwebenginecertificateerror.h
        qwebengineclientcertificateselection.cpp qwebengineclientcertificateselection.h
        qwebengineclientcertificatestore.cpp qwebengineclientcertificatestore.h
        qwebenginecontextmenurequest.cpp qwebenginecontextmenurequest.h qwebenginecontextmenurequest_p.h
        qwebenginecookiestore.cpp qwebenginecookiestore.h qwebenginecookiestore_p.h
        qwebenginedownloadrequest.cpp qwebenginedownloadrequest.h qwebenginedownloadrequest_p.h
        qwebenginefilesystemaccessrequest.cpp qwebenginefilesystemaccessrequest.h
        qwebenginefindtextresult.cpp qwebenginefindtextresult.h
        qwebenginefullscreenrequest.cpp qwebenginefullscreenrequest.h
        qwebenginehistory.cpp qwebenginehistory.h qwebenginehistory_p.h
        qwebenginehttprequest.cpp qwebenginehttprequest.h
        qwebengineloadinginfo.cpp qwebengineloadinginfo.h
        qwebenginemessagepumpscheduler.cpp qwebenginemessagepumpscheduler_p.h
        qwebenginenavigationrequest.cpp qwebenginenavigationrequest.h
        qwebenginenewwindowrequest.cpp qwebenginenewwindowrequest.h qwebenginenewwindowrequest_p.h
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
        ../../3rdparty/chromium/third_party/perfetto/include
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

if(CLANG OR GCC)
    target_compile_options(WebEngineCore PRIVATE "-Wno-unused-parameter")
endif()

qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_webchannel
    PUBLIC_LIBRARIES
        Qt::WebChannel
)
qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_geolocation
    PUBLIC_LIBRARIES
        Qt::Positioning
)

##
#  DOCS
##

qt_internal_add_docs(WebEngineCore
    ../doc/qtwebengine.qdocconf
)

add_custom_command(
    OUTPUT chromium_attributions.qdoc
    COMMAND ${Python3_EXECUTABLE} chromium/tools/licenses.py
        --file-template ../core/doc/about_credits.tmpl
        --entry-template ../core/doc/about_credits_entry.tmpl
        credits ${CMAKE_CURRENT_BINARY_DIR}/chromium_attributions.qdoc
    DEPENDS ../doc/about_credits.tmpl ../doc/about_credits_entry.tmpl
    WORKING_DIRECTORY ${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty
    USES_TERMINAL
)
add_custom_target(generate_chromium_attributions DEPENDS chromium_attributions.qdoc)
add_dependencies(generate_docs_WebEngineCore generate_chromium_attributions)

##
#  WEBENGINECORE RESOURCES
##

#TODO: use simply filter / globbing-expressions
set(localeList am ar bg bn ca cs da de el en-GB en-US es-419 es et fa fi fil fr
    gu he hi hr hu id it ja kn ko lt lv ml mr ms nb nl pl pt-BR pt-PT ro ru sk
    sl sr sv sw ta te th tr uk vi zh-CN zh-TW)

set(resourceList qtwebengine_resources.pak
    qtwebengine_resources_100p.pak
    qtwebengine_resources_200p.pak
    qtwebengine_devtools_resources.pak)

get_install_config(config)
get_architectures(archs)
list(GET archs 0 arch)

foreach(loc ${localeList})
    get_filename_component(locSourcePath ${buildDir}/${config}/${arch}/qtwebengine_locales/${loc}.pak REALPATH)
    list(APPEND localeFiles ${locSourcePath})
endforeach()

foreach(res ${resourceList})
    get_filename_component(resSourcePath ${buildDir}/${config}/${arch}/${res} REALPATH)
    list(APPEND resourceFiles ${resSourcePath})
endforeach()

if (NOT QT_FEATURE_webengine_system_icu)
    get_filename_component(icuFile ${buildDir}/${config}/${arch}/icudtl.dat REALPATH)
    list(APPEND resourceFiles ${icuFile})
    set_target_properties(WebEngineCore PROPERTIES ICUDTL_FILE ${icuFile})
endif()

if(QT_FEATURE_framework)
    set(allResourceFiles ${localeFiles} ${resourceFiles})
    target_sources(WebEngineCore PRIVATE ${allResourceFiles})

    set_source_files_properties(${localeFiles}
        TARGET_DIRECTORY WebEngineCore
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources/qtwebengine_locales
        GENERATED TRUE
    )
    set_source_files_properties(${resourceFiles}
        TARGET_DIRECTORY WebEngineCore
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
        GENERATED TRUE
    )

    add_custom_command(OUTPUT ${allResourceFiles} DEPENDS ${buildDir}/${config}/${arch}/QtWebEngineCore.stamp)
    add_custom_target(generate_resources_${config} DEPENDS ${allResourceFiles})

    addCopyCommand(WebEngineCore "${localeFiles}"
        "${QT_BUILD_DIR}/${INSTALL_LIBDIR}/QtWebEngineCore.framework/Versions/A/Resources/qtwebengine_locales/"
    )
    addCopyCommand(WebEngineCore "${resourceFiles}"
        "${QT_BUILD_DIR}/${INSTALL_LIBDIR}/QtWebEngineCore.framework/Versions/A/Resources/"
    )

else()
    install(FILES ${localeFiles}
        DESTINATION ${INSTALL_TRANSLATIONSDIR}/qtwebengine_locales
        CONFIGURATIONS ${config}
    )
    install(FILES ${resourceFiles}
        DESTINATION ${INSTALL_DATADIR}/resources
        CONFIGURATIONS ${config}
    )
    if(QT_SUPERBUILD OR NOT QT_WILL_INSTALL)
        addCopyCommand(WebEngineCore "${localeFiles}"
            ${QT_BUILD_DIR}/${INSTALL_TRANSLATIONSDIR}/qtwebengine_locales
        )
        addCopyCommand(WebEngineCore "${resourceFiles}"
            ${QT_BUILD_DIR}/${INSTALL_DATADIR}/resources
        )
   endif()
endif()
