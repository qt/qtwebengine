cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH)
endif()
if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/../.." REALPATH)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBENGINE_ROOT_SOURCE_DIR}/cmake")
set(buildDir "${CMAKE_CURRENT_BINARY_DIR}")

include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)
assertRunAsTopLevelBuild(TRUE)

add_subdirectory(api)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT REQUIRED)
find_package(Ninja 1.7.2 REQUIRED)
find_package(Python2 2.7.5 REQUIRED)
find_package(Nodejs 12.0)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(XSCRNSAVER xscrnsaver)
endif()

get_target_property(qtWebEngineProcessName WebEngineCore QTWEBENGINEPROCESS_NAME)
get_target_property(gnCmd Gn::gn IMPORTED_LOCATION)
get_qt_features(featureList webengine)

if(QT_GENERATOR_IS_MULTI_CONFIG)
    set(configs ${CMAKE_CONFIGURATION_TYPES})
else()
    set(configs ${CMAKE_BUILD_TYPE})
endif()

foreach(config ${configs})

##
#   BULID.gn SETUP
##

    unset(buildGn)
    set(buildGn buildGn_${config})

    add_gn_target(${buildGn} SOURCES DEFINES CXX_COMPILE_OPTIONS C_COMPILE_OPTIONS INCLUDES)

    resolve_target_includes(gnIncludes WebEngineCore)
    get_forward_declaration_macro(forwardDeclarationMacro)

    extend_gn_target(${buildGn}
        INCLUDES
            ${gnIncludes}
        DEFINES
            QT_NO_KEYWORDS
            QT_USE_QSTRINGBUILDER
            QTWEBENGINECORE_VERSION_STR=\\\\\"${QT_REPO_MODULE_VERSION}\\\\\"
            QTWEBENGINEPROCESS_NAME=\\\\\"${qtWebEngineProcessName}\\\\\"
            BUILDING_CHROMIUM
            ${forwardDeclarationMacro}
        SOURCES
            accessibility_activation_observer.cpp accessibility_activation_observer.h
            accessibility_tree_formatter_qt.cpp
            authentication_dialog_controller.cpp authentication_dialog_controller.h authentication_dialog_controller_p.h
            browser_accessibility_manager_qt.cpp browser_accessibility_manager_qt.h
            browser_accessibility_qt.cpp browser_accessibility_qt.h
            browser_main_parts_qt.cpp browser_main_parts_qt.h
            browser_message_filter_qt.cpp browser_message_filter_qt.h
            browsing_data_remover_delegate_qt.cpp browsing_data_remover_delegate_qt.h
            build_config_qt.h
            certificate_error_controller.cpp certificate_error_controller.h
            chromium_overrides.cpp
            client_cert_select_controller.cpp client_cert_select_controller.h
            clipboard_change_observer.h
            clipboard_qt.cpp clipboard_qt.h
            color_chooser_controller.cpp color_chooser_controller.h color_chooser_controller_p.h
            color_chooser_qt.cpp color_chooser_qt.h
            common/qt_ipc_logging.cpp
            common/qt_messages.cpp common/qt_messages.h
            compositor/compositor.cpp compositor/compositor.h
            compositor/content_gpu_client_qt.cpp compositor/content_gpu_client_qt.h
            compositor/display_overrides.cpp
            compositor/display_software_output_surface.cpp compositor/display_software_output_surface.h
            content_browser_client_qt.cpp content_browser_client_qt.h
            content_client_qt.cpp content_client_qt.h
            content_main_delegate_qt.cpp content_main_delegate_qt.h
            content_utility_client_qt.cpp content_utility_client_qt.h
            delegated_frame_host_client_qt.cpp delegated_frame_host_client_qt.h
            desktop_screen_qt.cpp desktop_screen_qt.h
            devtools_frontend_qt.cpp devtools_frontend_qt.h
            devtools_manager_delegate_qt.cpp devtools_manager_delegate_qt.h
            download_manager_delegate_qt.cpp download_manager_delegate_qt.h
            favicon_manager.cpp favicon_manager.h
            file_picker_controller.cpp file_picker_controller.h
            find_text_helper.cpp find_text_helper.h
            global_descriptors_qt.h
            javascript_dialog_controller.cpp javascript_dialog_controller.h javascript_dialog_controller_p.h
            javascript_dialog_manager_qt.cpp javascript_dialog_manager_qt.h
            login_delegate_qt.cpp login_delegate_qt.h
            media_capture_devices_dispatcher.cpp media_capture_devices_dispatcher.h
            native_web_keyboard_event_qt.cpp
            net/client_cert_override.cpp net/client_cert_override.h
            net/client_cert_store_data.cpp net/client_cert_store_data.h
            net/cookie_monster_delegate_qt.cpp net/cookie_monster_delegate_qt.h
            net/custom_url_loader_factory.cpp net/custom_url_loader_factory.h
            net/proxy_config_monitor.cpp
            net/proxy_config_service_qt.cpp
            net/proxying_restricted_cookie_manager_qt.cpp net/proxying_restricted_cookie_manager_qt.h
            net/proxying_url_loader_factory_qt.cpp net/proxying_url_loader_factory_qt.h
            net/qrc_url_scheme_handler.cpp net/qrc_url_scheme_handler.h
            net/ssl_host_state_delegate_qt.cpp net/ssl_host_state_delegate_qt.h
            net/system_network_context_manager.cpp net/system_network_context_manager.h
            net/url_request_custom_job_delegate.cpp net/url_request_custom_job_delegate.h
            net/url_request_custom_job_proxy.cpp net/url_request_custom_job_proxy.h
            net/webui_controller_factory_qt.cpp net/webui_controller_factory_qt.h
            ozone/gl_context_qt.cpp ozone/gl_context_qt.h
            ozone/gl_ozone_egl_qt.cpp ozone/gl_ozone_egl_qt.h
            ozone/gl_share_context_qt.cpp ozone/gl_share_context_qt.h
            ozone/gl_surface_egl_qt.cpp ozone/gl_surface_egl_qt.h
            ozone/gl_surface_qt.cpp ozone/gl_surface_qt.h
            ozone/gl_surface_wgl_qt.cpp ozone/gl_surface_wgl_qt.h
            ozone/platform_window_qt.cpp ozone/platform_window_qt.h
            ozone/surface_factory_qt.cpp ozone/surface_factory_qt.h
            permission_manager_qt.cpp permission_manager_qt.h
            platform_notification_service_qt.cpp platform_notification_service_qt.h
            pref_service_adapter.cpp pref_service_adapter.h
            process_main.cpp
            profile_adapter.cpp profile_adapter.h
            profile_adapter_client.cpp profile_adapter_client.h
            profile_io_data_qt.cpp profile_io_data_qt.h
            profile_qt.cpp profile_qt.h
            proxy_config_monitor.h
            proxy_config_service_qt.h
            quota_permission_context_qt.cpp quota_permission_context_qt.h
            quota_request_controller.h
            quota_request_controller_impl.cpp quota_request_controller_impl.h
            register_protocol_handler_request_controller.h
            register_protocol_handler_request_controller_impl.cpp register_protocol_handler_request_controller_impl.h
            render_view_context_menu_qt.cpp render_view_context_menu_qt.h
            render_widget_host_view_qt.cpp render_widget_host_view_qt.h
            render_widget_host_view_qt_delegate.h
            render_widget_host_view_qt_delegate_client.cpp render_widget_host_view_qt_delegate_client.h
            renderer/content_renderer_client_qt.cpp renderer/content_renderer_client_qt.h
            renderer/content_settings_observer_qt.cpp renderer/content_settings_observer_qt.h
            renderer/render_configuration.cpp renderer/render_configuration.h
            renderer/render_frame_observer_qt.cpp renderer/render_frame_observer_qt.h
            renderer/user_resource_controller.cpp renderer/user_resource_controller.h
            renderer/web_engine_page_render_frame.cpp renderer/web_engine_page_render_frame.h
            renderer_host/user_resource_controller_host.cpp renderer_host/user_resource_controller_host.h
            renderer_host/web_engine_page_host.cpp renderer_host/web_engine_page_host.h
            request_controller.h
            resource_bundle_qt.cpp
            resource_context_qt.cpp resource_context_qt.h
            touch_handle_drawable_client.h
            touch_handle_drawable_qt.cpp touch_handle_drawable_qt.h
            touch_selection_controller_client_qt.cpp touch_selection_controller_client_qt.h
            touch_selection_menu_controller.cpp touch_selection_menu_controller.h
            type_conversion.cpp type_conversion.h
            user_notification_controller.cpp user_notification_controller.h
            user_script.cpp user_script.h
            visited_links_manager_qt.cpp visited_links_manager_qt.h
            web_contents_adapter.cpp web_contents_adapter.h
            web_contents_adapter_client.h
            web_contents_delegate_qt.cpp web_contents_delegate_qt.h
            web_contents_view_qt.cpp web_contents_view_qt.h
            web_engine_context.cpp web_engine_context.h
            web_engine_context_threads.cpp
            web_engine_error.cpp web_engine_error.h
            web_engine_library_info.cpp web_engine_library_info.h
            web_engine_settings.cpp web_engine_settings.h
            web_event_factory.cpp web_event_factory.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_ozone_x11
        SOURCES
            ozone/gl_ozone_glx_qt.cpp ozone/gl_ozone_glx_qt.h
            ozone/gl_surface_glx_qt.cpp ozone/gl_surface_glx_qt.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_opengl
        SOURCES
            compositor/compositor_resource_fence.cpp compositor/compositor_resource_fence.h
            compositor/display_gl_output_surface.cpp compositor/display_gl_output_surface.h
    )

    extend_gn_target(${buildGn} CONDITION MACOS AND QT_FEATURE_opengl
        SOURCES
            macos_context_type_helper.mm macos_context_type_helper.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_pepper_plugins
        SOURCES
            renderer/pepper/pepper_renderer_host_factory_qt.cpp renderer/pepper/pepper_renderer_host_factory_qt.h
            renderer/plugins/loadable_plugin_placeholder_qt.cpp renderer/plugins/loadable_plugin_placeholder_qt.h
            renderer_host/pepper/pepper_host_factory_qt.cpp renderer_host/pepper/pepper_host_factory_qt.h
            renderer_host/pepper/pepper_isolated_file_system_message_filter.cpp renderer_host/pepper/pepper_isolated_file_system_message_filter.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_printing_and_pdf
        SOURCES
            printing/pdfium_document_wrapper_qt.cpp printing/pdfium_document_wrapper_qt.h
            printing/print_view_manager_base_qt.cpp printing/print_view_manager_base_qt.h
            printing/print_view_manager_qt.cpp printing/print_view_manager_qt.h
            printing/printer_worker.cpp printing/printer_worker.h
            printing/printing_message_filter_qt.cpp printing/printing_message_filter_qt.h
            renderer/print_web_view_helper_delegate_qt.cpp renderer/print_web_view_helper_delegate_qt.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_geolocation
        SOURCES
            location_provider_qt.cpp location_provider_qt.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_webchannel
        SOURCES
            renderer/web_channel_ipc_transport.cpp renderer/web_channel_ipc_transport.h
            renderer_host/web_channel_ipc_transport_host.cpp renderer_host/web_channel_ipc_transport_host.h
    )

    extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_extensions
        SOURCES
            common/extensions/extensions_api_provider_qt.cpp common/extensions/extensions_api_provider_qt.h
            common/extensions/extensions_client_qt.cpp common/extensions/extensions_client_qt.h
            extensions/component_extension_resource_manager_qt.cpp extensions/component_extension_resource_manager_qt.h
            extensions/extension_host_delegate_qt.cpp extensions/extension_host_delegate_qt.h
            extensions/extension_system_factory_qt.cpp extensions/extension_system_factory_qt.h
            extensions/extension_system_qt.cpp extensions/extension_system_qt.h
            extensions/extension_web_contents_observer_qt.cpp extensions/extension_web_contents_observer_qt.h
            extensions/extensions_api_client_qt.cpp extensions/extensions_api_client_qt.h
            extensions/extensions_browser_client_qt.cpp extensions/extensions_browser_client_qt.h
            extensions/messaging_delegate_qt.cpp extensions/messaging_delegate_qt.h
            extensions/mime_handler_view_guest_delegate_qt.cpp extensions/mime_handler_view_guest_delegate_qt.h
            extensions/pdf_iframe_navigation_throttle_qt.cpp extensions/pdf_iframe_navigation_throttle_qt.h
            extensions/plugin_service_filter_qt.cpp extensions/plugin_service_filter_qt.h
            net/plugin_response_interceptor_url_loader_throttle.cpp net/plugin_response_interceptor_url_loader_throttle.h
            renderer/extensions/extensions_dispatcher_delegate_qt.cpp renderer/extensions/extensions_dispatcher_delegate_qt.h
            renderer/extensions/extensions_renderer_client_qt.cpp renderer/extensions/extensions_renderer_client_qt.h
            renderer/extensions/renderer_permissions_policy_delegate_qt.cpp renderer/extensions/renderer_permissions_policy_delegate_qt.h
            renderer/extensions/resource_request_policy_qt.cpp renderer/extensions/resource_request_policy_qt.h
    )

    read_gn_target(${buildGn} ${WEBENGINE_ROOT_BUILD_DIR}/src/core/api/${config}/gn_config.cxx.cmake)
    read_gn_target(${buildGn} ${WEBENGINE_ROOT_BUILD_DIR}/src/core/api/${config}/gn_config.c.cmake)
    configure_gn_target(${buildGn} ${config}
        ${WEBENGINE_ROOT_SOURCE_DIR}/src/core/configure/BUILD.root.gn.in ${buildDir}/${config}/BUILD.gn)

##
#   TOOLCHAIN SETUP
##

    configure_file(${WEBENGINE_ROOT_SOURCE_DIR}/src/core/configure/BUILD.toolchain.gn.in
        ${buildDir}/${config}/toolchain/BUILD.gn @ONLY)

##
#   GN CALL PARAMETERS SETUP
##

    unset(gnArg)
    unset(gnArgArg)
    set(gnArg gen ${buildDir}/${config})

    list(APPEND gnArg
        --script-executable=${Python2_EXECUTABLE}
        --root=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium)

    if(${config} STREQUAL "Debug")
        list(APPEND gnArgArg is_debug=true symbol_level=2)
        if(WIN32 AND NOT CLANG)
            list(APPEND gnArgArg enable_iterator_debugging=true v8_optimized_debug=false)
        endif()
    elseif(${config} STREQUAL "Release")
        list(APPEND gnArgArg is_debug=false symbol_level=0)
    elseif(${config} STREQUAL "RelWithDebInfo")
        list(APPEND gnArgArg is_debug=false)
        if(WIN32 AND NOT CLANG)
            list(APPEND gnArgArg symbol_level=2)
        else()
            list(APPEND gnArgArg symbol_level=1)
        endif()
    elseif(${config} STREQUAL "MinSizeRel")
        list(APPEND gnArgArg is_debug=false symbol_level=0 optimize_for_size=true)
    endif()

    list(APPEND gnArgArg
        qtwebengine_target="${buildDir}/${config}:QtWebEngineCore"
        use_qt=true
        init_stack_vars=false
        is_component_build=false
        is_shared=true
        enable_debugallocation=false
        enable_media_remoting=false
        enable_message_center=false
        enable_nacl=false
        enable_remoting=false
        enable_reporting=false
        enable_resource_allowlist_generation=false
        enable_swiftshader=false
        enable_swiftshader_vulkan=false
        angle_enable_swiftshader=false
        enable_web_speech=false
        enable_widevine=true
        forbid_non_component_debug_builds=false
        has_native_accessibility=false
        safe_browsing_mode=0
        skia_use_dawn=false
        toolkit_views=false
        treat_warnings_as_errors=false
        use_allocator_shim=false
        use_allocator="none"
        use_custom_libcxx=false
        chrome_pgo_phase=0
        enable_hangout_services_extension=false
        optimize_webui=false
        enable_js_type_check=false
        v8_use_external_startup_data=false
        strip_absolute_paths_from_debug_symbols=false
        enable_precompiled_headers=false
        is_official_build=false
        is_unsafe_developer_build=false
        from_here_uses_location_builtins=false
        use_debug_fission=false
        blink_symbol_level=0
        remove_v8base_debug_symbols=true
    )
    extend_gn_list(gnArgArg
        ARGS use_jumbo_build
        CONDITION NOT WIN32
    )
    if(NOT WIN32)
        list(APPEND gnArgArg jumbo_file_merge_limit=8 jumbo_build_excluded=["browser"])
    endif()
    extend_gn_list(gnArgArg
        ARGS is_clang
        CONDITION CLANG
    )
    if(CLANG)
        list(APPEND gnArgArg clang_use_chrome_plugins=false)
    endif()
    extend_gn_list(gnArgArg
        ARGS use_gold
        CONDITION QT_FEATURE_use_gold_linker
    )
    extend_gn_list(gnArgArg
        ARGS use_lld
        CONDITION QT_FEATURE_use_lld_linker
    )
    extend_gn_list(gnArgArg
        ARGS enable_basic_printing enable_print_preview enable_pdf
        CONDITION QT_FEATURE_webengine_printing_and_pdf
    )
    extend_gn_list(gnArgArg
        ARGS enable_plugins
        CONDITION QT_FEATURE_webengine_pepper_plugins
    )
    extend_gn_list(gnArgArg
        ARGS enable_spellcheck
        CONDITION QT_FEATURE_webengine_spellchecker
    )
    extend_gn_list(gnArgArg
        ARGS enable_webrtc
        CONDITION QT_FEATURE_webengine_webrtc
    )
    extend_gn_list(gnArgArg
        ARGS rtc_use_pipewire
        CONDITION QT_FEATURE_webengine_webrtc_pipewire
    )
    extend_gn_list(gnArgArg
        ARGS enable_extensions
        CONDITION QT_FEATURE_webengine_extensions
    )
    extend_gn_list(gnArgArg
        ARGS use_kerberos
        CONDITION QT_FEATURE_webengine_kerberos
    )
    extend_gn_list(gnArgArg
        ARGS proprietary_codecs
        CONDITION QT_FEATURE_webengine_proprietary_codecs
    )
    if(QT_FEATURE_webengine_proprietary_codecs)
        list(APPEND gnArgArg ffmpeg_branding="Chrome")
    endif()
    extend_gn_list(gnArgArg
        ARGS use_browser_spellchecker
        CONDITION QT_FEATURE_webengine_native_spellchecker
    )
    extend_gn_list(gnArgArg
        ARGS use_rollup
        CONDITION TARGET Nodejs::Nodejs
    )
    if(LINUX)
        list(APPEND gnArgArg
            use_cups=false
            use_gio=false
            use_gnome_keyring=false
            use_udev=true
            use_bundled_fontconfig=false
            use_sysroot=false
            enable_session_service=false
            is_cfi=false
            use_ozone=true
            use_x11=false
            ozone_auto_platforms=false
            ozone_platform_headless=false
            ozone_platform_external=true
            ozone_platform="qt"
            ozone_extra_path="${CMAKE_CURRENT_LIST_DIR}/ozone/ozone_extra.gni"
            custom_toolchain="${buildDir}/${config}/toolchain:target"
            host_toolchain="${buildDir}/${config}/toolchain:host"
            host_cpu="x64"
            pkg_config="pkg-config"
            host_pkg_config="/usr/bin/pkg-config"
            use_glib=false
        )
        set(systemLibs libjpeg libpng freetype harfbuzz libevent libwebp libxml
            opus snappy libvpx icu ffmpeg re2 lcms2
        )
        foreach(slib ${systemLibs})
            extend_gn_list(gnArgArg
                ARGS use_system_${slib}
                CONDITION QT_FEATURE_webengine_system_${slib}
            )
        endforeach()
        extend_gn_list(gnArgArg
            ARGS icu_use_data_file
            CONDITION NOT QT_FEATURE_webengine_system_icu
        )
        extend_gn_list(gnArgArg
            ARGS use_system_zlib use_system_minizip
            CONDITION QT_FEATURE_webengine_system_zlib AND QT_FEATURE_webengine_system_minizip
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_system_zlib
            CONDITION QT_FEATURE_webengine_system_zlib
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_system_libpng
            CONDITION QT_FEATURE_webengine_system_libpng
        )
        extend_gn_list(gnArgArg
            ARGS use_libpci
            CONDITION QT_FEATURE_webengine_system_libpci
        )
        extend_gn_list(gnArgArg
            ARGS use_alsa
            CONDITION QT_FEATURE_webengine_system_alsa
        )
        extend_gn_list(gnArgArg
            ARGS use_pulseaudio
            CONDITION QT_FEATURE_webengine_system_pulseaudio
        )
        extend_gn_list(gnArgArg
            ARGS ozone_platform_x11 use_xkbcommon
            CONDITION QT_FEATURE_webengine_ozone_x11
        )
        extend_gn_list(gnArgArg
            ARGS rtc_use_x11
            CONDITION QT_FEATURE_webengine_ozone_x11 AND QT_FEATURE_webengine_webrtc
        )
        extend_gn_list(gnArgArg
            ARGS use_xscrnsaver
            CONDITION XSCRNSAVER_FOUND
        )
    endif()

    if(MACOS)
        get_darwin_sdk_version(macSdkVersion)
        get_filename_component(clangBasePath ${CMAKE_OBJCXX_COMPILER} DIRECTORY)
        get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
        list(APPEND gnArgArg
            use_sysroot=false
            use_system_xcode=true
            clang_base_path="${clangBasePath}"
            clang_use_chrome_plugins=false
            mac_deployment_target="${CMAKE_OSX_DEPLOYMENT_TARGET}"
            mac_sdk_min="${macSdkVersion}"
            use_external_popup_menu=false
            angle_enable_vulkan=false
       )
    endif()

    if(WIN32)
        get_filename_component(windowsSdkPath $ENV{WINDOWSSDKDIR} DIRECTORY)
        get_filename_component(visualStudioPath $ENV{VSINSTALLDIR} DIRECTORY)
        list(APPEND gnArgArg
            use_sysroot=false
            enable_session_service=false
            ninja_use_custom_environment_files=false
            win_linker_timing=true
            com_init_check_hook_disabled=true
            heterogeneous_executables=true
            enable_vr=false
            use_incremental_linking=false
            visual_studio_version=2019
            visual_studio_path=\"${visualStudioPath}\"
            windows_sdk_path=\"${windowsSdkPath}\"
            target_cpu="x64"
        )
    endif()

    list(JOIN gnArgArg " " gnArgArg)
    list(APPEND gnArg "--args=${gnArgArg}")

##
#  GN CALL
##

    list(JOIN gnArg " " printArg)
    message("-- Running ${config} Configuration for GN \n-- ${gnCmd} ${printArg}")
    execute_process(
        COMMAND ${gnCmd} ${gnArg}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE gnResult
        OUTPUT_VARIABLE gnOutput
        ERROR_VARIABLE gnError
    )

    if(NOT gnResult EQUAL 0)
        message(FATAL_ERROR "\n-- GN FAILED\n${gnOutput}\n${gnError}")
    else()
        string(REGEX REPLACE "\n$" "" gnOutput "${gnOutput}")
        message("-- GN ${gnOutput}")
    endif()
    include(${buildDir}/${config}/QtWebEngineCore.cmake)


##
#  NINJA CALL
##

    # config loop is a workaround to be able to add_custom_command per config
    # note this is fixed in CMAKE.3.20 and should be cleaned up when 3.20 is
    # the minimum cmake we support

    if(WIN32)
        set(sandboxOutput ${buildDir}/${config}/QtWebEngineCoreSandbox.lib)
        set(sandboxTarget QtWebEngineCoreSandbox)
    else()
        unset(sandboxOutput)
        unset(sandboxTarget)
    endif()

    add_custom_command(
        OUTPUT ${buildDir}/${config}/QtWebEngineCore.stamp ${sandboxOutput} runAlways # use generator expression in CMAKE 3.20
        WORKING_DIRECTORY ${buildDir}/${config}
        COMMAND ${CMAKE_COMMAND} -E echo "Ninja ${config} build"
        COMMAND Ninja::ninja
            $ENV{NINJAFLAGS}
            -C ${buildDir}/${config}
            QtWebEngineCore ${sandboxTarget}
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(ninja_${config} DEPENDS ${buildDir}/${config}/QtWebEngineCore.stamp ${sandboxOutput} runAlways)
endforeach()

##
#  WEBENGINECORE SETUP
##

target_include_directories(WebEngineCore PRIVATE ${buildDir}/$<CONFIG>/gen)

if(WIN32)
    add_library(WebEngineCoreSandbox STATIC IMPORTED GLOBAL)
    add_dependencies(WebEngineCoreSandbox ninja)
endif()

foreach(config ${configs})

     string(TOUPPER ${config} cfg)

     add_library(GnObjects_${config} OBJECT IMPORTED GLOBAL)
     target_link_libraries(WebEngineCore PRIVATE $<$<CONFIG:${config}>:GnObjects_${config}>)
     add_dependencies(GnObjects_${config} ninja_${config})
     #TODO: remove GnObjects_${config} with CMAKE 3.20
     set_property(TARGET GnObjects_${config} PROPERTY IMPORTED_OBJECTS_${cfg} ${${cfg}_NINJA_OBJECTS})
     set_source_files_properties(${${cfg}_NINJA_OBJECTS} PROPERTIES GENERATED TRUE)

     if(LINUX)
     target_link_libraries(WebEngineCore
         PRIVATE "-Wl,--start-group" "$<$<CONFIG:${config}>:${${cfg}_NINJA_ARCHIVES}>" "-Wl,--end-group")
     else()
     target_link_libraries(WebEngineCore PRIVATE "$<$<CONFIG:${config}>:${${cfg}_NINJA_ARCHIVES}>")
     endif()

     target_link_libraries(WebEngineCore PUBLIC  "$<$<CONFIG:${config}>:${${cfg}_NINJA_LIBS}>")

     # we depend on WebEnigneCore stamp, but ninja backend generator needs more
     add_custom_command(OUTPUT ${${cfg}_NINJA_OBJECTS} ${${cfg}_NINJA_ARCHIVES}
         DEPENDS ${buildDir}/${config}/QtWebEngineCore.stamp)
     add_custom_target(generate_${cfg} DEPENDS ${${cfg}_NINJA_OBJECTS} ${${cfg}_NINJA_ARCHIVES})

     if(WIN32)
         set(sandboxLibraryPath ${buildDir}/${config}/QtWebEngineCoreSandbox.lib)
         set_property(TARGET WebEngineCoreSandbox
             PROPERTY IMPORTED_LOCATION_${cfg} ${sandboxLibraryPath})
         set_source_files_properties(${sandboxLibraryPath} PROPERTIES GENERATED TRUE)
         add_custom_command(OUTPUT ${sandboxLibraryPath} DEPENDS ninja_${config})
         add_custom_target(generate_sandbox_${cfg} DEPENDS ${sandboxLibraryPath})
     endif()

endforeach()

##
#  RESOURCES
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

foreach(loc ${localeList})
    get_filename_component(locSourcePath ${buildDir}/${config}/qtwebengine_locales/${loc}.pak REALPATH)
    list(APPEND localeFiles ${locSourcePath})
endforeach()

foreach(res ${resourceList})
    get_filename_component(resSourcePath ${buildDir}/${config}/${res} REALPATH)
    list(APPEND resourceFiles ${resSourcePath})
endforeach()

get_filename_component(icuFiles ${buildDir}/${config}/icudtl.dat REALPATH)

if(QT_FEATURE_framework)
    set(allResourceFiles ${localeFiles} ${resourceFiles} ${icuFiles})
    target_sources(WebEngineCore PRIVATE ${icuFiles} ${resourceFiles} ${localeFiles})

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
    set_source_files_properties(${icuFiles}
        TARGET_DIRECTORY WebEngineCore
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
        GENERATED TRUE
    )
    add_custom_command(OUTPUT ${localeFiles} ${resourceFiles} ${icuFiles}
        DEPENDS ${buildDir}/${config}/QtWebEngineCore.stamp
    )
    add_custom_target(generate_resources_${config}
        DEPENDS ${localeFiles} ${resourcesFiles} ${icuFiles}
    )
else()
    install(FILES ${localeFiles}
        DESTINATION translations/qtwebengine_locales
        CONFIGURATIONS ${config}
    )
    install(FILES ${resourceFiles}
        DESTINATION resources
        CONFIGURATIONS ${config}
    )
    install(FILES ${icuFiles}
        DESTINATION resources
        CONFIGURATIONS ${config}
    )
# install (again) for superbuild
    install(FILES ${localeFiles}
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/translations/qtwebengine_locales
        CONFIGURATIONS ${config}
    )
    install(FILES ${resourceFiles}
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/resources
        CONFIGURATIONS ${config}
    )
    install(FILES ${icuFiles}
        DESTINATION ${WEBENGINE_ROOT_BUILD_DIR}/resources
        CONFIGURATIONS ${config}
    )
    if(NOT QT_WILL_INSTALL)
        # TODO: Could we get rid of ALL?
        add_custom_target(
                install_resources_${config}
                ALL
                DEPENDS ${buildDir}/${config}/QtWebEngineCore.stamp
                COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_INSTALL_PREFIX}/resources
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${resourceFiles} ${icuFiles}
                        ${CMAKE_INSTALL_PREFIX}/resources)
        add_custom_target(
                install_locales_${config}
                ALL
                DEPENDS ${buildDir}/${config}/QtWebEngineCore.stamp
                COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_INSTALL_PREFIX}/translations/qtwebengine_locales
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${localeFiles}
                        ${CMAKE_INSTALL_PREFIX}/translations/qtwebengine_locales)
    endif()
endif()
