# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gyp_generator.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gyp_generator $$CONFIG
GYPFILE = $$PWD/core_generated.gyp
GYPINCLUDES += qtwebengine.gypi

TEMPLATE = lib

# NOTE: The TARGET, QT, QT_PRIVATE variables must match those in core_module.pro.
# gyp/ninja will take care of the compilation, qmake/make will finish with linking and install.
TARGET = QtWebEngineCore
QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private

# Defining keywords such as 'signal' clashes with the chromium code base.
DEFINES += QT_NO_KEYWORDS \
           Q_FORWARD_DECLARE_OBJC_CLASS=QT_FORWARD_DECLARE_CLASS \
           QTWEBENGINEPROCESS_NAME=\\\"$$QTWEBENGINEPROCESS_NAME\\\" \
           QTWEBENGINECORE_VERSION_STR=\\\"$$MODULE_VERSION\\\" \
           BUILDING_CHROMIUM

# Assume that we want mobile touch and low-end hardware behaviors
# whenever we are cross compiling.
cross_compile: DEFINES += QTWEBENGINE_MOBILE_SWITCHES

# Keep Skia happy
CONFIG(release, debug|release): DEFINES += NDEBUG

contains(QT_CONFIG, egl): CONFIG += egl
else: DEFINES += QT_NO_EGL

RESOURCES += devtools.qrc

# something fishy with qmake in 5.2 ?
INCLUDEPATH += $$[QT_INSTALL_HEADERS] $$PWD

SOURCES = \
        browser_context_qt.cpp \
        chromium_gpu_helper.cpp \
        chromium_overrides.cpp \
        clipboard_qt.cpp \
        common/qt_messages.cpp \
        content_client_qt.cpp \
        content_browser_client_qt.cpp \
        content_main_delegate_qt.cpp \
        delegated_frame_node.cpp \
        desktop_screen_qt.cpp \
        dev_tools_http_handler_delegate_qt.cpp \
        download_manager_delegate_qt.cpp \
        gl_context_qt.cpp \
        javascript_dialog_controller.cpp \
        javascript_dialog_manager_qt.cpp \
        media_capture_devices_dispatcher.cpp \
        process_main.cpp \
        qt_render_view_observer_host.cpp \
        render_widget_host_view_qt.cpp \
        renderer/content_renderer_client_qt.cpp \
        renderer/qt_render_view_observer.cpp \
        resource_bundle_qt.cpp \
        resource_context_qt.cpp \
        resource_dispatcher_host_delegate_qt.cpp \
        stream_video_node.cpp \
        url_request_context_getter_qt.cpp \
        web_contents_adapter.cpp \
        web_contents_delegate_qt.cpp \
        web_contents_view_qt.cpp \
        web_engine_context.cpp \
        web_engine_error.cpp \
        web_engine_library_info.cpp \
        web_event_factory.cpp \
        yuv_video_node.cpp \
        qrc_protocol_handler_qt.cpp \
        url_request_qrc_job_qt.cpp \
        surface_factory_qt.cpp \
        ozone_platform_eglfs.cpp


HEADERS = \
        browser_context_qt.h \
        chromium_overrides.h \
        clipboard_qt.h \
        common/qt_messages.h \
        content_client_qt.h \
        content_browser_client_qt.h \
        content_main_delegate_qt.h \
        delegated_frame_node.h \
        desktop_screen_qt.h \
        dev_tools_http_handler_delegate_qt.h \
        download_manager_delegate_qt.h \
        chromium_gpu_helper.h \
        gl_context_qt.h \
        javascript_dialog_controller_p.h \
        javascript_dialog_controller.h \
        javascript_dialog_manager_qt.h \
        media_capture_devices_dispatcher.h \
        process_main.h \
        qt_render_view_observer_host.h \
        render_widget_host_view_qt.h \
        render_widget_host_view_qt_delegate.h \
        renderer/content_renderer_client_qt.h \
        renderer/qt_render_view_observer.h \
        resource_context_qt.h \
        resource_dispatcher_host_delegate_qt.h \
        stream_video_node.h \
        url_request_context_getter_qt.h \
        web_contents_adapter.h \
        web_contents_adapter_client.h \
        web_contents_delegate_qt.h \
        web_contents_view_qt.h \
        web_engine_context.h \
        web_engine_error.h \
        web_engine_library_info.h \
        web_event_factory.h \
        yuv_video_node.h \
        qrc_protocol_handler_qt.h \
        url_request_qrc_job_qt.h \
        surface_factory_qt.h \
        ozone_platform_eglfs.h
