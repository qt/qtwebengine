# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gyp_generator.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gyp_generator $$CONFIG
GYPDEPENDENCIES += <(chromium_src_dir)/content/browser/devtools/devtools_resources.gyp:devtools_resources
GYPINCLUDES += ../qtwebengine.gypi

TEMPLATE = lib

TARGET = Qt5WebEngineCore

# gyp sets the default install name to /usr/local/lib and we need the module libraries to
# know its install_name so that they can let the dynamic linker load the core library.
# FIXME: Remove this and put it in qtwebengine.gypi once we can use a relative path to @loader_path.
macx: GYP_DYLIB_INSTALL_NAME_BASE = $$getOutDir()/$$getConfigDir()

QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private
qtHaveModule(v8): QT_PRIVATE += v8-private

COPY_FILES = <(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak
COPY_DESTINATIONS = ../resources/

# Defining keywords such as 'signal' clashes with the chromium code base.
DEFINES += QT_NO_KEYWORDS \
           Q_FORWARD_DECLARE_OBJC_CLASS=QT_FORWARD_DECLARE_CLASS

# We need a way to tap into gyp´s Debug vs. Release configuration
PER_CONFIG_DEFINES = QTWEBENGINEPROCESS_PATH=\\\"$$getOutDir()/%config/$$QTWEBENGINEPROCESS_NAME\\\"

# Keep Skia happy
CONFIG(release, debug|release): DEFINES += NDEBUG

RESOURCES += lib_resources.qrc devtools.qrc
# We need this to find the include files generated for the .pak resource files.
INCLUDEPATH += $$absolute_path(../resources, $$PWD)

# something fishy with qmake in 5.2 ?
INCLUDEPATH += $$[QT_INSTALL_HEADERS]

SOURCES = \
        backing_store_qt.cpp \
        chromium_overrides.cpp \
        browser_accessibility_manager_qt.cpp \
        browser_accessibility_qt.cpp \
        content_client_qt.cpp \
        content_browser_client_qt.cpp \
        delegated_frame_node.cpp \
        dev_tools_http_handler_delegate_qt.cpp \
        download_manager_delegate_qt.cpp \
        chromium_gpu_helper.cpp \
        javascript_dialog_manager_qt.cpp \
        process_main.cpp \
        render_widget_host_view_qt.cpp \
        resource_bundle_qt.cpp \
        resource_context_qt.cpp \
        url_request_context_getter_qt.cpp \
        web_contents_adapter.cpp \
        web_contents_delegate_qt.cpp \
        web_contents_view_qt.cpp \
        web_engine_context.cpp \
        web_event_factory.cpp \
        yuv_video_node.cpp

HEADERS = \
        backing_store_qt.h \
        browser_accessibility_manager_qt.h \
        browser_accessibility_qt.h \
        browser_context_qt.h \
        chromium_overrides.h \
        content_client_qt.h \
        content_browser_client_qt.h \
        delegated_frame_node.h \
        dev_tools_http_handler_delegate_qt.h \
        download_manager_delegate_qt.h \
        chromium_gpu_helper.h \
        javascript_dialog_manager_qt.h \
        process_main.h \
        render_widget_host_view_qt.h \
        render_widget_host_view_qt_delegate.h \
        resource_context_qt.h \
        url_request_context_getter_qt.h \
        web_contents_adapter.h \
        web_contents_adapter_client.h \
        web_contents_delegate_qt.h \
        web_contents_view_qt.h \
        web_engine_context.h \
        web_event_factory.h \
        yuv_video_node.h

