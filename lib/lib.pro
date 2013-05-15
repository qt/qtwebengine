# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gypi_gen.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gypi_gen $$CONFIG

TEMPLATE = lib

TARGET = blinq

# Defining keywords such as 'signal' clashes with the chromium code base.
DEFINES += QT_NO_KEYWORDS \
           BLINQ_PROCESS_PATH=\\\"$$getOutDir()/$$BLINQ_PROCESS_NAME\\\"

# Keep Skia happy
CONFIG(release, debug|release): DEFINES += NDEBUG

QT += gui-private widgets

SOURCES = \
        backing_store_qt.cpp \
        blinqpage.cpp \
        raster_window.cpp \
        render_widget_host_view_qt.cpp \
        shell_qt.cpp \
        signal_connector.cpp

HEADERS = \
        backing_store_qt.h \
        blinqpage.h \
        raster_window.h \
        render_widget_host_view_qt.h \
        signal_connector.h

