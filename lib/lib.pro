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
        blinqpage.cpp

HEADERS = \
        blinqpage.h

