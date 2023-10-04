# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.19)

find_package(Ninja 1.7.2 REQUIRED)
find_package(Nodejs 14.19 REQUIRED)
find_package(Perl)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    create_pkg_config_host_wrapper(${CMAKE_CURRENT_BINARY_DIR})
endif()

set(buildDir "${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(api)
add_subdirectory(tools)

##
#   TOOLCHAIN SETUP
##

if(LINUX)
    setup_toolchains()
endif()

##
#   GN BUILD SETUP
##

addSyncTargets(core)

get_configs(configs)
get_architectures(archs)

foreach(arch ${archs})
    foreach(config ${configs})

        ##
        # BUILD.gn SETUP
        ##

        set(buildGn core_${config}_${arch})
        add_gn_target(${buildGn} ${config} ${arch}
            SOURCES DEFINES CXX_COMPILE_OPTIONS C_COMPILE_OPTIONS INCLUDES MOC_PATH
        )
        resolve_target_includes(gnIncludes WebEngineCore)
        get_forward_declaration_macro(forwardDeclarationMacro)
        get_target_property(qtWebEngineProcessName WebEngineCore QTWEBENGINEPROCESS_NAME)

        if(QT_FEATURE_debug_and_release AND ("${config}" STREQUAL "Debug"))
            set(qtWebEngineProcessName "${qtWebEngineProcessName}${CMAKE_DEBUG_POSTFIX}")
        endif()

        set(gnCxxCompileOptions "")
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            # Silence warning in boringssl headers.
            list(APPEND gnCxxCompileOptions -Wno-ignored-qualifiers)

            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.2.0")
                # GCC 11.1 has a regression in the integrated preprocessor, so disable it as a workaround (QTBUG-93360)
                # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100796
                list(APPEND gnCxxCompileOptions -no-integrated-cpp)
            endif()
        endif()

        extend_gn_target(${buildGn}
            INCLUDES
                ${gnIncludes}
            DEFINES
                QT_NO_KEYWORDS
                QT_USE_QSTRINGBUILDER
                QTWEBENGINECORE_VERSION_STR=${QT_REPO_MODULE_VERSION}
                QTWEBENGINEPROCESS_NAME=${qtWebEngineProcessName}
                BUILDING_CHROMIUM
                "${forwardDeclarationMacro}"
            CXX_COMPILE_OPTIONS
                ${gnCxxCompileOptions}
            SOURCES
                accessibility_tree_formatter_qt.cpp
                authentication_dialog_controller.cpp authentication_dialog_controller.h authentication_dialog_controller_p.h
                autofill_client_qt.cpp autofill_client_qt.h
                autofill_popup_controller.cpp autofill_popup_controller.h autofill_popup_controller_p.h
                browser_accessibility_manager_qt.cpp browser_accessibility_manager_qt.h
                browser_main_parts_qt.cpp browser_main_parts_qt.h
                browser_message_filter_qt.cpp browser_message_filter_qt.h
                browsing_data_remover_delegate_qt.cpp browsing_data_remover_delegate_qt.h
                build_config_qt.h
                certificate_error_controller.cpp certificate_error_controller.h
                chromium_overrides.cpp
                client_cert_select_controller.cpp client_cert_select_controller.h
                client_hints.cpp client_hints.h
                clipboard_change_observer.h
                clipboard_qt.cpp clipboard_qt.h
                color_chooser_controller.cpp color_chooser_controller.h color_chooser_controller_p.h
                color_chooser_qt.cpp color_chooser_qt.h
                common/qt_messages.cpp common/qt_messages.h
                compositor/compositor.cpp compositor/compositor.h
                compositor/content_gpu_client_qt.cpp compositor/content_gpu_client_qt.h
                compositor/display_overrides.cpp
                compositor/display_software_output_surface.cpp compositor/display_software_output_surface.h
                compositor/native_skia_output_device.cpp compositor/native_skia_output_device.h
                content_browser_client_qt.cpp content_browser_client_qt.h
                content_client_qt.cpp content_client_qt.h
                content_main_delegate_qt.cpp content_main_delegate_qt.h
                content_utility_client_qt.cpp content_utility_client_qt.h
                custom_handlers/protocol_handler_registry_delegate_qt.h custom_handlers/protocol_handler_registry_delegate_qt.cpp
                custom_handlers/protocol_handler_registry_factory.h custom_handlers/protocol_handler_registry_factory.cpp
                custom_handlers/register_protocol_handler_request_controller.h
                custom_handlers/register_protocol_handler_request_controller_impl.cpp custom_handlers/register_protocol_handler_request_controller_impl.h
                delegated_frame_host_client_qt.cpp delegated_frame_host_client_qt.h
                desktop_screen_qt.cpp desktop_screen_qt.h
                devtools_frontend_qt.cpp devtools_frontend_qt.h
                devtools_manager_delegate_qt.cpp devtools_manager_delegate_qt.h
                download_manager_delegate_qt.cpp download_manager_delegate_qt.h
                favicon_driver_qt.cpp favicon_driver_qt.h
                favicon_service_factory_qt.cpp favicon_service_factory_qt.h
                file_picker_controller.cpp file_picker_controller.h
                file_system_access/file_system_access_permission_context_factory_qt.cpp file_system_access/file_system_access_permission_context_factory_qt.h
                file_system_access/file_system_access_permission_context_qt.cpp file_system_access/file_system_access_permission_context_qt.h
                file_system_access/file_system_access_permission_grant_qt.cpp file_system_access/file_system_access_permission_grant_qt.h
                file_system_access/file_system_access_permission_request_controller.h
                file_system_access/file_system_access_permission_request_controller_impl.cpp file_system_access/file_system_access_permission_request_controller_impl.h
                file_system_access/file_system_access_permission_request_manager_qt.cpp file_system_access/file_system_access_permission_request_manager_qt.h
                find_text_helper.cpp find_text_helper.h
                global_descriptors_qt.h
                javascript_dialog_controller.cpp javascript_dialog_controller.h javascript_dialog_controller_p.h
                javascript_dialog_manager_qt.cpp javascript_dialog_manager_qt.h
                login_delegate_qt.cpp login_delegate_qt.h
                media_capture_devices_dispatcher.cpp media_capture_devices_dispatcher.h
                native_web_keyboard_event_qt.cpp
                net/client_cert_qt.cpp net/client_cert_qt.h
                net/client_cert_store_data.cpp net/client_cert_store_data.h
                net/cookie_monster_delegate_qt.cpp net/cookie_monster_delegate_qt.h
                net/custom_url_loader_factory.cpp net/custom_url_loader_factory.h
                net/proxy_config_monitor.cpp net/proxy_config_monitor.h
                net/proxy_config_service_qt.cpp net/proxy_config_service_qt.h
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
                pointer_device_qt.cpp
                pref_service_adapter.cpp pref_service_adapter.h
                process_main.cpp
                profile_adapter.cpp profile_adapter.h
                profile_adapter_client.cpp profile_adapter_client.h
                profile_io_data_qt.cpp profile_io_data_qt.h
                profile_qt.cpp profile_qt.h
                render_view_context_menu_qt.cpp render_view_context_menu_qt.h
                render_widget_host_view_qt.cpp render_widget_host_view_qt.h
                render_widget_host_view_qt_delegate.h
                render_widget_host_view_qt_delegate_client.cpp render_widget_host_view_qt_delegate_client.h
                render_widget_host_view_qt_delegate_item.cpp render_widget_host_view_qt_delegate_item.h
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
                select_file_dialog_factory_qt.cpp select_file_dialog_factory_qt.h
                touch_handle_drawable_client.h
                touch_handle_drawable_qt.cpp touch_handle_drawable_qt.h
                touch_selection_controller_client_qt.cpp touch_selection_controller_client_qt.h
                touch_selection_menu_controller.cpp touch_selection_menu_controller.h
                type_conversion.cpp type_conversion.h
                user_notification_controller.cpp user_notification_controller.h
                user_script.cpp user_script.h
                version_ui_qt.cpp version_ui_qt.h
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
                web_usb_detector_qt.cpp web_usb_detector_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_accessibility
            SOURCES
                accessibility_activation_observer.cpp accessibility_activation_observer.h
                browser_accessibility_qt.cpp browser_accessibility_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_ozone_x11
            SOURCES
                ozone/gl_ozone_glx_qt.cpp ozone/gl_ozone_glx_qt.h
                ozone/gl_surface_glx_qt.cpp ozone/gl_surface_glx_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_vulkan
            SOURCES
                compositor/vulkan_implementation_qt.cpp compositor/vulkan_implementation_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_opengl
            SOURCES
                compositor/compositor_resource_fence.cpp compositor/compositor_resource_fence.h
                compositor/display_skia_output_device.cpp compositor/display_skia_output_device.h
        )

        extend_gn_target(${buildGn} CONDITION MACOS
            SOURCES
                compositor/native_skia_output_device_mac.mm
                compositor/native_skia_output_device_mac2.mm
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_pepper_plugins
            SOURCES
                renderer/pepper/pepper_renderer_host_factory_qt.cpp renderer/pepper/pepper_renderer_host_factory_qt.h
                renderer/plugins/loadable_plugin_placeholder_qt.cpp renderer/plugins/loadable_plugin_placeholder_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_printing_and_pdf
            SOURCES
                printing/pdfium_document_wrapper_qt.cpp printing/pdfium_document_wrapper_qt.h
                printing/print_view_manager_base_qt.cpp printing/print_view_manager_base_qt.h
                printing/print_view_manager_qt.cpp printing/print_view_manager_qt.h
                printing/printer_worker.cpp printing/printer_worker.h
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
                extensions/file_system_delegate_qt.cpp extensions/file_system_delegate_qt.h
                extensions/messaging_delegate_qt.cpp extensions/messaging_delegate_qt.h
                extensions/mime_handler_view_guest_delegate_qt.cpp extensions/mime_handler_view_guest_delegate_qt.h
                extensions/plugin_service_filter_qt.cpp extensions/plugin_service_filter_qt.h
                net/plugin_response_interceptor_url_loader_throttle.cpp net/plugin_response_interceptor_url_loader_throttle.h
                renderer/extensions/extensions_dispatcher_delegate_qt.cpp renderer/extensions/extensions_dispatcher_delegate_qt.h
                renderer/extensions/extensions_renderer_client_qt.cpp renderer/extensions/extensions_renderer_client_qt.h
                renderer/extensions/renderer_permissions_policy_delegate_qt.cpp renderer/extensions/renderer_permissions_policy_delegate_qt.h
                renderer/extensions/resource_request_policy_qt.cpp renderer/extensions/resource_request_policy_qt.h
        )

        extend_gn_target(${buildGn} CONDITION QT_FEATURE_webengine_extensions AND QT_FEATURE_webengine_printing_and_pdf
            SOURCES
                extensions/pdf_iframe_navigation_throttle_qt.cpp extensions/pdf_iframe_navigation_throttle_qt.h
                printing/pdf_stream_delegate_qt.cpp printing/pdf_stream_delegate_qt.h
                printing/pdf_web_contents_helper_client_qt.cpp printing/pdf_web_contents_helper_client_qt.h
        )

        extend_gn_target(${buildGn} CONDITION WIN32
            SOURCES
                clipboard_util_win.cpp
        )

        ##
        #   GN PARAMETERS SETUP
        ##

        unset(gnArgArg)

        append_build_type_setup(gnArgArg)
        append_compiler_linker_sdk_setup(gnArgArg)
        append_sanitizer_setup(gnArgArg)
        append_toolchain_setup(gnArgArg)
        append_pkg_config_setup(gnArgArg)

        list(APPEND gnArgArg
            qtwebengine_target="${buildDir}/${config}/${arch}:QtWebEngineCore"
            build_with_tflite_lib=false
            enable_background_contents=false
            enable_background_mode=false
            enable_ipc_fuzzer=false
            enable_ipc_logging=false
            enable_media_remoting=false
            enable_message_center=false
            enable_nacl=false
            enable_oop_printing=false
            enable_remoting=false
            enable_reporting=false
            enable_resource_allowlist_generation=false
            enable_screen_ai_service=false
            enable_session_service=false
            enable_supervised_users=false
            enable_swiftshader=false
            enable_swiftshader_vulkan=false
            angle_enable_swiftshader=false
            dawn_use_swiftshader=false
            use_dawn=false
            build_dawn_tests=false
            enable_vr=false
            enable_web_speech=false
            enable_widevine=true
            enable_library_cdms=true
            fatal_linker_warnings=false
            has_native_accessibility=false
            safe_browsing_mode=0
            skia_use_dawn=false
            toolkit_views=false
            chrome_pgo_phase=0
            optimize_webui=false
            strip_absolute_paths_from_debug_symbols=false
            pdf_use_skia=true
            devtools_fast_bundle=false
            devtools_skip_typecheck=false
            use_static_angle=true
            chrome_root_store_supported=false
            trial_comparison_cert_verifier_supported=false
        )
        extend_gn_list(gnArgArg
            ARGS use_v8_context_snapshot v8_use_external_startup_data
            CONDITION QT_FEATURE_webengine_v8_context_snapshot
        )
        extend_gn_list(gnArgArg
            ARGS enable_printing enable_basic_printing enable_print_preview enable_pdf
            CONDITION QT_FEATURE_webengine_printing_and_pdf
        )
        extend_gn_list(gnArgArg
            ARGS use_cups
            CONDITION QT_FEATURE_webengine_printing_and_pdf AND NOT WIN32
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
            ARGS enable_screen_capture
            CONDITION QT_FEATURE_webengine_webrtc
        )
        extend_gn_list(gnArgArg
            ARGS enable_hangout_services_extension
            CONDITION QT_FEATURE_webengine_webrtc AND QT_FEATURE_webengine_extensions
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
            ARGS enable_vulkan
            CONDITION QT_FEATURE_webengine_vulkan
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
            ARGS use_embedded_config
            CONDITION QT_FEATURE_webengine_embedded_build
        )

        if(LINUX)
            list(APPEND gnArgArg
                use_gtk=false # GTK toolkit bindings
                use_qt=false # Qt5 toolkit bindings
                use_cups=false
                use_gio=false
                use_gnome_keyring=false
                use_bundled_fontconfig=false
                use_glib=false
                use_bluez=false
                use_udev=true
                is_cfi=false
                use_ozone=true
                ozone_auto_platforms=false
                ozone_platform_headless=false
                ozone_platform_external=true
                ozone_platform="qt"
                ozone_extra_path="${CMAKE_CURRENT_LIST_DIR}/ozone/ozone_extra.gni"
            )
            set(systemLibs libjpeg libpng freetype harfbuzz libevent libwebp libxml
                opus snappy icu ffmpeg re2 lcms2 libopenjpeg2 libvpx
            )
            foreach(slib ${systemLibs})
                extend_gn_list(gnArgArg
                    ARGS use_system_${slib}
                    CONDITION QT_FEATURE_webengine_system_${slib}
                )
            endforeach()
            if(NOT QT_FEATURE_webengine_system_opus)
                extend_gn_list(gnArgArg
                   ARGS has_perl
                   CONDITION Perl_FOUND
            )
            endif()
            extend_gn_list(gnArgArg
                ARGS use_system_libxslt
                CONDITION QT_FEATURE_webengine_system_libxml
            )
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
                ARGS pdfium_use_system_libtiff
                CONDITION QT_FEATURE_webengine_system_libtiff
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
                ARGS use_vaapi
                CONDITION QT_FEATURE_webengine_vaapi
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
                ARGS use_vaapi_x11
                CONDITION QT_FEATURE_webengine_ozone_x11 AND QT_FEATURE_webengine_vaapi
            )

            if(QT_FEATURE_webengine_kerberos)
                list(APPEND gnArgArg
                     external_gssapi_include_dir="${GSSAPI_INCLUDE_DIRS}/gssapi"
                )
            endif()

            if(CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm")
                check_thumb(armThumb)
                if(NOT armThumb AND NOT QT_FEATURE_webengine_system_ffmpeg)
                    list(APPEND gnArgArg media_use_ffmpeg=false use_webaudio_ffmpeg=false)
                endif()
            endif()

            if(CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm64")
                # This is a workaround to avoid auto test timeouts on the QEMU arm64 CI.
                if ("$ENV{TARGET_OSVERSION_COIN}" STREQUAL "qemu")
                    list(APPEND gnArgArg
                        v8_enable_sandbox=false
                        arm_control_flow_integrity="none"
                    )
                endif()
            endif()
        endif()

        if(MACOS)
            list(APPEND gnArgArg
                use_external_popup_menu=false
            )
        endif()

        if(NOT CLANG)
            list(APPEND gnArgArg
                enable_location_source=false)
        endif()

        if(WIN32)
            list(APPEND gnArgArg
                ninja_use_custom_environment_files=false
                com_init_check_hook_disabled=true
                heterogeneous_executables=true
                use_static_angle=true
            )
        endif()

        add_gn_command(
            CMAKE_TARGET WebEngineCore
            NINJA_TARGETS QtWebEngineCore convert_dict
            GN_TARGET ${buildGn}
            GN_ARGS ${gnArgArg}
            BUILDDIR ${buildDir}/${config}/${arch}
            MODULE core
        )

    endforeach()
    create_cxx_configs(WebEngineCore ${arch})
endforeach()


##
#  WEBENGINECORE SETUP
##

list(GET archs 0 arch)
target_include_directories(WebEngineCore PRIVATE
    ${buildDir}/$<CONFIG>/${arch}/gen
    ${buildDir}/$<CONFIG>/${arch}/gen/third_party/perfetto
    ${buildDir}/$<CONFIG>/${arch}/gen/third_party/perfetto/build_config
)

set(stamps QtWebEngineCore.stamp)
if(QT_FEATURE_webengine_v8_context_snapshot)
    set(dataStamp obj/tools/v8_context_snapshot/v8_context_snapshot.stamp)
endif()

add_gn_build_aritfacts_to_target(
    CMAKE_TARGET WebEngineCore
    NINJA_TARGET QtWebEngineCore
    MODULE core
    BUILDDIR ${buildDir}
    COMPLETE_STATIC FALSE
    NINJA_STAMP QtWebEngineCore.stamp
    NINJA_DATA_STAMP ${dataStamp}
)
add_dependencies(WebEngineCore run_core_NinjaDone)
if(COIN_BUG_699)
   set_property(TARGET WebEngineCore PROPERTY CXX_LINKER_LAUNCHER ${PROJECT_BINARY_DIR}/linker_ulimit.sh)
endif()

##
#  SANDBOX
##

if(WIN32)
    add_library(WebEngineCoreSandbox STATIC IMPORTED GLOBAL)
    foreach(config ${configs})
        string(TOUPPER ${config} cfg)
        add_ninja_command(
            TARGET QtWebEngineCoreSandbox
            OUTPUT QtWebEngineCoreSandbox.lib
            BUILDDIR ${buildDir}/${config}/${arch}
            MODULE core
        )
        set(sandboxLibraryPath ${buildDir}/${config}/${arch}/QtWebEngineCoreSandbox.lib)
        set_property(TARGET WebEngineCoreSandbox
            PROPERTY IMPORTED_LOCATION_${cfg} ${sandboxLibraryPath}
        )
        add_custom_target(sandboxLibrary_${config}
            DEPENDS ${buildDir}/${config}/${arch}/QtWebEngineCoreSandbox.lib)
        add_dependencies(run_core_NinjaDone sandboxLibrary_${config})
        add_dependencies(WebEngineCoreSandbox WebEngineCore)
    endforeach()
endif()

##
#  WEBENGINECORE DICT TOOL SETUP
##

if(QT_FEATURE_webengine_spellchecker AND NOT CMAKE_CROSSCOMPILING)
    qt_get_tool_target_name(dict_target_name qwebengine_convert_dict)
    target_include_directories(${dict_target_name} PRIVATE
        ../3rdparty/chromium
        ../3rdparty/chromium/third_party/boringssl/src/include
        ${buildDir}/$<CONFIG>/${arch}/gen
    )
    add_gn_build_aritfacts_to_target(
        CMAKE_TARGET ${dict_target_name}
        NINJA_TARGET convert_dict
        MODULE core
        BUILDDIR ${buildDir}
        COMPLETE_STATIC FALSE
        NINJA_STAMP convert_dict.stamp
    )
    add_dependencies(${dict_target_name} run_core_NinjaDone)
    add_dependencies(${dict_target_name} WebEngineCore)
endif()

##
#  CHROMIUM UPDATE
##

add_custom_target(update-chromium
    COMMAND ${CMAKE_COMMAND} -P ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/SubmoduleUpdate.cmake
    DEPENDS ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/SubmoduleUpdate.cmake
)
