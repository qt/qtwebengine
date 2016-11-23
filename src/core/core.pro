TEMPLATE = subdirs

# core_headers is a dummy module to syncqt the headers so we can
# use them by later targets
core_headers.file = core_headers.pro
core_api.file = api/core_api.pro

# This will take the compile output of ninja, and link+deploy the final binary.
core_module.file = core_module.pro
core_module.depends = core_api

# core_generator.pro is a dummy .pro file that is used by qmake
# to generate our main .gyp/BUILD.gn file
core_generator.file = core_generator.pro
core_generator.depends = core_headers

use?(gn) {
    # core_gn_generator.pro is a dummy .pro file that is used by qmake
    # to generate our main BUILD.gn file

    core_icu.files = $$OUT_PWD/$$getConfigDir()/icudtl.dat
    core_icu.path = $$[QT_INSTALL_DATA]/resources
    core_icu.CONFIG += no_check_exist

    core_locales.files = $$OUT_PWD/$$getConfigDir()/qtwebengine_locales/*.pak
    core_locales.path = $$[QT_INSTALL_TRANSLATIONS]/qtwebengine_locales
    core_locales.CONFIG += no_check_exist

    core_resources.files = \
        $$OUT_PWD/$$getConfigDir()/qtwebengine_resources.pak \
        $$OUT_PWD/$$getConfigDir()/qtwebengine_resources_100p.pak \
        $$OUT_PWD/$$getConfigDir()/qtwebengine_resources_200p.pak \
        $$OUT_PWD/$$getConfigDir()/qtwebengine_devtools_resources.pak
    core_resources.path = $$[QT_INSTALL_DATA]/resources
    core_resources.CONFIG += no_check_exist
    INSTALLS += core_resources core_locales core_icu

    gn_run.file = gn_run.pro
    gn_run.depends = core_generator

    core_api.depends = gn_run

    SUBDIRS += \
               core_api \
               core_headers \
               core_generator \
               core_module \
               gn_run
} else {

    # gyp_run.pro calls gyp through gyp_qtwebengine on the qmake step, and ninja on the make step.
    gyp_run.file = gyp_run.pro
    gyp_run.depends = core_generator
    core_api.depends = gyp_run

    SUBDIRS += gyp_run \
               core_api \
               core_module \
               core_headers \
               core_generator

    !win32 {
        # gyp_configure_host.pro and gyp_configure_target.pro are phony pro files that
        # extract things like compiler and linker from qmake
        # Do not use them on Windows, where Qt already expects the toolchain to be
        # selected through environment varibles.
        gyp_configure_host.file = gyp_configure_host.pro
        gyp_configure_target.file = gyp_configure_target.pro
        gyp_configure_target.depends = gyp_configure_host

        gyp_run.depends += gyp_configure_host gyp_configure_target
        SUBDIRS += gyp_configure_host gyp_configure_target
    }
}
