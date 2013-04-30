# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gypi_gen.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gypi_gen $$CONFIG

TARGET = $$BLINQ_PROCESS_NAME
TEMPLATE = app

QT -= gui core

SOURCES = main.cpp
