set(GN_DEFINES QT_NO_KEYWORDS
      QT_USE_QSTRINGBUILDER
      QTWEBENGINECORE_VERSION_STR=\\\"${QT_REPO_MODULE_VERSION}\\\"
      QTWEBENGINEPROCESS_NAME=\\\"${QTWEBENGINEPROCESS_NAME}\\\"
      BUILDING_CHROMIUM)

set(GN_SOURCES_IN
      accessibility_activation_observer.cpp
      accessibility_tree_formatter_qt.cpp
      authentication_dialog_controller.cpp
      browser_accessibility_manager_qt.cpp
      browser_accessibility_qt.cpp
      browsing_data_remover_delegate_qt.cpp
      browser_main_parts_qt.cpp
      browser_message_filter_qt.cpp
      certificate_error_controller.cpp
      chromium_overrides.cpp
      client_cert_select_controller.cpp
      clipboard_qt.cpp
      color_chooser_qt.cpp
      color_chooser_controller.cpp
      common/qt_ipc_logging.cpp
      common/qt_messages.cpp
      compositor/compositor.cpp
      compositor/content_gpu_client_qt.cpp
      compositor/display_overrides.cpp
      compositor/display_software_output_surface.cpp
      content_client_qt.cpp
      content_browser_client_qt.cpp
      content_main_delegate_qt.cpp
      content_utility_client_qt.cpp
      delegated_frame_host_client_qt.cpp
      desktop_screen_qt.cpp
      devtools_frontend_qt.cpp
      devtools_manager_delegate_qt.cpp
      download_manager_delegate_qt.cpp
      favicon_manager.cpp
      file_picker_controller.cpp
      find_text_helper.cpp
      javascript_dialog_controller.cpp
      javascript_dialog_manager_qt.cpp
      login_delegate_qt.cpp
      media_capture_devices_dispatcher.cpp
      native_web_keyboard_event_qt.cpp
      net/client_cert_override.cpp
      net/client_cert_store_data.cpp
      net/cookie_monster_delegate_qt.cpp
      net/custom_url_loader_factory.cpp
      net/proxy_config_monitor.cpp
      net/proxy_config_service_qt.cpp
      net/proxying_url_loader_factory_qt.cpp
      net/proxying_restricted_cookie_manager_qt.cpp
      net/qrc_url_scheme_handler.cpp
      net/ssl_host_state_delegate_qt.cpp
      net/system_network_context_manager.cpp
      net/url_request_custom_job_delegate.cpp
      net/url_request_custom_job_proxy.cpp
      net/webui_controller_factory_qt.cpp
      ozone/gl_context_qt.cpp
      ozone/gl_share_context_qt.cpp
      ozone/gl_ozone_egl_qt.cpp
      ozone/gl_surface_qt.cpp
      ozone/gl_surface_egl_qt.cpp
      ozone/gl_surface_wgl_qt.cpp
      ozone/platform_window_qt.cpp
      ozone/surface_factory_qt.cpp
      permission_manager_qt.cpp
      platform_notification_service_qt.cpp
      process_main.cpp
      profile_adapter.cpp
      profile_adapter_client.cpp
      profile_qt.cpp
      profile_io_data_qt.cpp
      quota_permission_context_qt.cpp
      quota_request_controller_impl.cpp
      pref_service_adapter.cpp
      register_protocol_handler_request_controller_impl.cpp
      render_view_context_menu_qt.cpp
      render_widget_host_view_qt.cpp
      render_widget_host_view_qt_delegate_client.cpp
      renderer/content_renderer_client_qt.cpp
      renderer/content_settings_observer_qt.cpp
      renderer/render_frame_observer_qt.cpp
      renderer/web_engine_page_render_frame.cpp
      renderer/render_configuration.cpp
      renderer/user_resource_controller.cpp
      renderer/plugins/plugin_placeholder_qt.cpp
      renderer_host/web_engine_page_host.cpp
      renderer_host/user_resource_controller_host.cpp
      resource_bundle_qt.cpp
      resource_context_qt.cpp
      touch_handle_drawable_qt.cpp
      touch_selection_controller_client_qt.cpp
      touch_selection_menu_controller.cpp
      type_conversion.cpp
      user_notification_controller.cpp
      user_script.cpp
      visited_links_manager_qt.cpp
      web_contents_adapter.cpp
      web_contents_delegate_qt.cpp
      web_contents_view_qt.cpp
      web_engine_context.cpp
      web_engine_context_threads.cpp
      web_engine_error.cpp
      web_engine_library_info.cpp
      web_engine_settings.cpp
      web_event_factory.cpp
      ozone/gl_surface_glx_qt.cpp
      ozone/gl_ozone_glx_qt.cpp
      compositor/compositor_resource_fence.cpp
      compositor/display_gl_output_surface.cpp)

set(GN_HEADERS_IN
      accessibility_activation_observer.h
      authentication_dialog_controller_p.h
      authentication_dialog_controller.h
      build_config_qt.h
      browser_accessibility_manager_qt.h
      browser_accessibility_qt.h
      browsing_data_remover_delegate_qt.h
      browser_main_parts_qt.h
      browser_message_filter_qt.h
      certificate_error_controller.h
      client_cert_select_controller.h
      clipboard_change_observer.h
      clipboard_qt.h
      color_chooser_qt.h
      color_chooser_controller_p.h
      color_chooser_controller.h
      common/qt_messages.h
      compositor/compositor.h
      compositor/content_gpu_client_qt.h
      compositor/display_software_output_surface.h
      content_client_qt.h
      content_browser_client_qt.h
      content_main_delegate_qt.h
      content_utility_client_qt.h
      delegated_frame_host_client_qt.h
      desktop_screen_qt.h
      devtools_frontend_qt.h
      devtools_manager_delegate_qt.h
      download_manager_delegate_qt.h
      favicon_manager.h
      file_picker_controller.h
      find_text_helper.h
      global_descriptors_qt.h
      javascript_dialog_controller_p.h
      javascript_dialog_controller.h
      javascript_dialog_manager_qt.h
      login_delegate_qt.h
      media_capture_devices_dispatcher.h
      net/client_cert_override.h
      net/client_cert_store_data.h
      net/cookie_monster_delegate_qt.h
      net/custom_url_loader_factory.h
      net/proxying_url_loader_factory_qt.h
      net/proxying_restricted_cookie_manager_qt.h
      net/qrc_url_scheme_handler.h
      net/ssl_host_state_delegate_qt.h
      net/system_network_context_manager.h
      net/url_request_custom_job_delegate.h
      net/url_request_custom_job_proxy.h
      net/webui_controller_factory_qt.h
      ozone/gl_context_qt.h
      ozone/gl_share_context_qt.h
      ozone/gl_ozone_egl_qt.h
      ozone/gl_surface_qt.h
      ozone/gl_surface_egl_qt.h
      ozone/gl_surface_wgl_qt.h
      ozone/platform_window_qt.h
      ozone/surface_factory_qt.h
      permission_manager_qt.h
      platform_notification_service_qt.h
      pref_service_adapter.h
      process_main.h
      profile_adapter.h
      profile_adapter_client.h
      profile_qt.h
      profile_io_data_qt.h
      proxy_config_monitor.h
      proxy_config_service_qt.h
      quota_permission_context_qt.h
      quota_request_controller.h
      quota_request_controller_impl.h
      register_protocol_handler_request_controller.h
      register_protocol_handler_request_controller_impl.h
      render_view_context_menu_qt.h
      render_widget_host_view_qt.h
      render_widget_host_view_qt_delegate.h
      render_widget_host_view_qt_delegate_client.h
      renderer/content_renderer_client_qt.h
      renderer/content_settings_observer_qt.h
      renderer/render_frame_observer_qt.h
      renderer/web_engine_page_render_frame.h
      renderer/render_configuration.h
      renderer/user_resource_controller.h
      renderer/plugins/plugin_placeholder_qt.h
      renderer_host/web_engine_page_host.h
      renderer_host/user_resource_controller_host.h
      request_controller.h
      resource_context_qt.h
      touch_handle_drawable_client.h
      touch_handle_drawable_qt.h
      touch_selection_controller_client_qt.h
      touch_selection_menu_controller.h
      type_conversion.h
      user_notification_controller.h
      user_script.h
      visited_links_manager_qt.h
      web_contents_adapter.h
      web_contents_adapter_client.h
      web_contents_delegate_qt.h
      web_contents_view_qt.h
      web_engine_context.h
      web_engine_error.h
      web_engine_library_info.h
      web_engine_settings.h
      web_event_factory.h
      ozone/gl_ozone_glx_qt.h
      ozone/gl_surface_glx_qt.h
      compositor/compositor_resource_fence.h
      compositor/display_gl_output_surface.h)

foreach(GN_HEADER_FILE ${GN_HEADERS_IN})
    get_filename_component(GN_HEADER_PATH ${GN_HEADER_FILE} ABSOLUTE)
    list(APPEND GN_HEADERS \"${GN_HEADER_PATH}\")
endforeach()
string(REPLACE ";" ",\n  " GN_HEADERS "${GN_HEADERS}")
foreach(GN_SOURCE_FILE ${GN_SOURCES_IN})
    get_filename_component(GN_SOURCE_PATH ${GN_SOURCE_FILE} ABSOLUTE)
    list(APPEND GN_SOURCES \"${GN_SOURCE_PATH}\")
endforeach()
string(REPLACE ";" ",\n  " GN_SOURCES "${GN_SOURCES}")
include(${CMAKE_WEBENGINE_ROOT_BUILD_PATH}/src/core/api/gn_config.cxx.cmake)
include(${CMAKE_WEBENGINE_ROOT_BUILD_PATH}/src/core/api/gn_config.c.cmake)
list(APPEND GN_DEFINES_IN ${GN_DEFINES})
list(REMOVE_DUPLICATES GN_DEFINES_IN)
unset(GN_DEFINES)
foreach(GN_DEFINE ${GN_DEFINES_IN})
    list(APPEND GN_ARGS_DEFINES \"-D${GN_DEFINE}\")
    list(APPEND GN_DEFINES \"${GN_DEFINE}\")
endforeach()
string(REPLACE ";" ",\n  " GN_ARGS_DEFINES "${GN_ARGS_DEFINES}")
string(REPLACE ";" ",\n  " GN_DEFINES "${GN_DEFINES}")
list(REMOVE_DUPLICATES GN_INCLUDES_IN)
foreach(GN_INCLUDE ${GN_INCLUDES_IN})
    list(APPEND GN_ARGS_INCLUDES \"-I${GN_INCLUDE}\")
    list(APPEND GN_INCLUDE_DIRS \"${GN_INCLUDE}\")
endforeach()
string(REPLACE ";" ",\n  " GN_ARGS_INCLUDES "${GN_ARGS_INCLUDES}")
string(REPLACE ";" ",\n  " GN_INCLUDE_DIRS "${GN_INCLUDE_DIRS}")
get_target_property(GN_MOC_BIN_IN Qt6::moc IMPORTED_LOCATION)
set(GN_ARGS_MOC_BIN \"${GN_MOC_BIN_IN}\")
foreach(GN_CXX_COMPILE_OPTION ${GN_CXX_COMPILE_OPTIONS_IN})
    list(APPEND GN_CFLAGS_CC \"${GN_CXX_COMPILE_OPTION}\")
endforeach()
list(REMOVE_DUPLICATES GN_CFLAGS_CC)
string(REPLACE ";" ",\n  " GN_CFLAGS_CC "${GN_CFLAGS_CC}")
foreach(GN_C_COMPILE_OPTION ${GN_C_COMPILE_OPTIONS_IN})
    list(APPEND GN_CFLAGS_C \"${GN_C_COMPILE_OPTION}\")
endforeach()
list(REMOVE_DUPLICATES GN_CFLAGS_C)
string(REPLACE ";" ",\n  " GN_CFLAGS_C "${GN_CFLAGS_C}")
