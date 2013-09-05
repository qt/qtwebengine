# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gyp_generator.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gyp_generator $$CONFIG
GYPDEPENDENCIES += ../shared/shared.gyp:qtwebengine_shared
GYPINCLUDES += ../qtwebengine.gypi

TEMPLATE = lib

TARGET = Qt5WebEngineCore

# Defining keywords such as 'signal' clashes with the chromium code base.
DEFINES += QT_NO_KEYWORDS

# We need a way to tap into gyp´s Debug vs. Release configuration
PER_CONFIG_DEFINES = QTWEBENGINEPROCESS_PATH=\\\"$$getOutDir()/%config/$$QTWEBENGINEPROCESS_NAME\\\"

# Keep Skia happy
CONFIG(release, debug|release): DEFINES += NDEBUG

RESOURCES += lib_resources.qrc
# We need this to find the include files generated for the .pak resource files.
INCLUDEPATH += $$absolute_path(../resources, $$PWD)

SOURCES = \
        backing_store_qt.cpp \
        content_browser_client_qt.cpp \
        download_manager_delegate_qt.cpp \
        javascript_dialog_manager_qt.cpp \
        render_widget_host_view_qt.cpp \
        render_widget_host_view_qt_delegate.cpp \
        resource_context_qt.cpp \
        url_request_context_getter_qt.cpp \
        web_contents_adapter.cpp \
        web_contents_delegate_qt.cpp \
        web_contents_view_qt.cpp \
        web_engine_context.cpp \
        web_event_factory.cpp

HEADERS = \
        backing_store_qt.h \
        browser_context_qt.h \
        content_browser_client_qt.h \
        download_manager_delegate_qt.h \
        javascript_dialog_manager_qt.h \
        render_widget_host_view_qt.h \
        render_widget_host_view_qt_delegate.h \
        resource_context_qt.h \
        url_request_context_getter_qt.h \
        web_contents_adapter.h \
        web_contents_adapter_client.h \
        web_contents_delegate_qt.h \
        web_contents_view_qt.h \
        web_engine_context.h \
        web_event_factory.h

