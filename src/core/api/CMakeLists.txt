# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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
        qwebengineurlrequestinterceptor.h qwebengineurlrequestinterceptor.cpp
        qwebengineurlrequestjob.cpp qwebengineurlrequestjob.h
        qwebengineurlscheme.cpp qwebengineurlscheme.h
        qwebengineurlschemehandler.cpp qwebengineurlschemehandler.h
        qwebengineglobalsettings.cpp qwebengineglobalsettings.h qwebengineglobalsettings_p.h
    DEFINES
        BUILDING_CHROMIUM
    INCLUDE_DIRECTORIES
        ../
        ../../3rdparty/chromium
        ../../3rdparty/chromium/third_party/abseil-cpp
        ../../3rdparty/chromium/third_party/perfetto/include
        ../../3rdparty/chromium/third_party/boringssl/src/include
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
    target_compile_options(WebEngineCore PRIVATE
        "-Wno-unused-parameter"
        "-Wno-expansion-to-defined"
    )
endif()

if(GCC)
    target_compile_options(WebEngineCore PRIVATE
        "-Wno-packed-not-aligned"
    )
endif()

qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_webchannel
    PUBLIC_LIBRARIES
        Qt::WebChannel
)
qt_internal_extend_target(WebEngineCore CONDITION QT_FEATURE_webengine_geolocation
    PUBLIC_LIBRARIES
        Qt::Positioning
)

get_install_config(config)
get_architectures(archs)
get_configs(configs)
list(GET archs 0 arch)

##
#  DOCS
##

qt_internal_add_docs(WebEngineCore
    ../doc/qtwebengine.qdocconf
)

add_code_attributions_target(
    TARGET generate_chromium_attributions
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/chromium_attributions.qdoc
    GN_TARGET :QtWebEngineCore
    FILE_TEMPLATE ../doc/about_credits.tmpl
    ENTRY_TEMPLATE ../doc/about_credits_entry.tmpl
    BUILDDIR ${buildDir}/${config}/${arch}
)
add_dependencies(generate_chromium_attributions run_core_GnDone)
add_dependencies(prepare_docs_WebEngineCore generate_chromium_attributions)

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

set(stamps ${buildDir}/${config}/${arch}/QtWebEngineCore.stamp)

qt_internal_get_filename_path_mode(path_mode)

if(QT_FEATURE_webengine_v8_context_snapshot)
    foreach(arch ${archs})
       foreach(config ${configs})
           if(MACOS)
               set(ext_arch ".${arch}")
           else()
               unset(ext_arch)
           endif()
           if("${config}" STREQUAL "Debug")
               set(ext_debug ".debug")
           else()
               unset(ext_debug)
           endif()
           get_filename_component(resSourcePath ${buildDir}/${config}/${arch}/v8_context_snapshot${ext_arch}${ext_debug}.bin ${path_mode})
           list(APPEND resourceFiles ${resSourcePath})
           if(MACOS)
               set(stamps ${stamps} ${buildDir}/${config}/${arch}/obj/tools/v8_context_snapshot/v8_context_snapshot.stamp)
           endif()
       endforeach()
    endforeach()
endif()

foreach(loc ${localeList})
    get_filename_component(locSourcePath ${buildDir}/${config}/${arch}/qtwebengine_locales/${loc}.pak ${path_mode})
    list(APPEND localeFiles ${locSourcePath})
endforeach()

foreach(res ${resourceList})
    get_filename_component(resSourcePath ${buildDir}/${config}/${arch}/${res} ${path_mode})
    list(APPEND resourceFiles ${resSourcePath})
endforeach()


if(NOT QT_FEATURE_webengine_system_icu)
    get_filename_component(icuFile ${buildDir}/${config}/${arch}/icudtl.dat ${path_mode})
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

    add_custom_command(OUTPUT ${allResourceFiles} DEPENDS "${stamps}")
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
