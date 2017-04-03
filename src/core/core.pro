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

# A fake project for qt creator
core_project.file = core_project.pro
core_project.depends = gn_run

SUBDIRS += \
            core_headers \
            core_generator \
            gn_run \
            core_api \
            core_module

false: SUBDIRS += core_project
