TEMPLATE = subdirs

# core_gyp_generator.pro is a dummy .pro file that is used by qmake
# to generate our main .gyp file
core_gyp_generator.file = core_gyp_generator.pro

# gyp_configure_host.pro and gyp_configure_target.pro are phony pro files that
# extract things like compiler and linker from qmake
gyp_configure_host.file = gyp_configure_host.pro
gyp_configure_target.file = gyp_configure_target.pro
gyp_configure_target.depends = gyp_configure_host

# gyp_run.pro calls gyp through gyp_qtwebengine on the qmake step, and ninja on the make step.
gyp_run.file = gyp_run.pro

gyp_run.depends = resources core_gyp_generator gyp_configure_host gyp_configure_target

SUBDIRS += core_gyp_generator \
           gyp_configure_host \
           gyp_configure_target \
           gyp_run \
           resources \
