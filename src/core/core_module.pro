MODULE = webenginecore

include(core_common.pri)
# Needed to set a CFBundleIdentifier
QMAKE_INFO_PLIST = Info_mac.plist

# Look for linking information produced by gyp for our target according to core_generated.gyp
!include($$OUT_PWD/$$getConfigDir()/$${TARGET}_linking.pri) {
    error("Could not find the linking information that gyp should have generated.")
}

load(qt_module)

api_library_name = qtwebenginecoreapi$$qtPlatformTargetSuffix()
api_library_path = $$OUT_PWD/api/$$getConfigDir()


LIBS_PRIVATE += -L$$api_library_path
CONFIG *= no_smart_library_merge
osx {
    LIBS_PRIVATE += -Wl,-force_load,$${api_library_path}$${QMAKE_DIR_SEP}lib$${api_library_name}.a
} else:msvc {
    # Simulate -whole-archive by passing the list of object files that belong to the public
    # API library as response file to the linker.
    QMAKE_LFLAGS += /OPT:REF
    QMAKE_LFLAGS += @$${api_library_path}$${QMAKE_DIR_SEP}$${api_library_name}.lib.objects
} else {
    LIBS_PRIVATE += -Wl,-whole-archive -l$$api_library_name -Wl,-no-whole-archive
}

win32-msvc* {
    POST_TARGETDEPS += $${api_library_path}$${QMAKE_DIR_SEP}$${api_library_name}.lib
} else {
    POST_TARGETDEPS += $${api_library_path}$${QMAKE_DIR_SEP}lib$${api_library_name}.a
}

# Using -Wl,-Bsymbolic-functions seems to confuse the dynamic linker
# and doesn't let Chromium get access to libc symbols through dlsym.
CONFIG -= bsymbolic_functions

contains(QT_CONFIG, egl): CONFIG += egl

linux: contains(QT_CONFIG, separate_debug_info): QMAKE_POST_LINK="cd $(DESTDIR) && $(STRIP) --strip-unneeded $(TARGET)"

REPACK_DIR = $$OUT_PWD/$$getConfigDir()/gen/repack
# Duplicated from resources/resources.gyp
LOCALE_LIST = am ar bg bn ca cs da de el en-GB en-US es-419 es et fa fi fil fr gu he hi hr hu id it ja kn ko lt lv ml mr ms nb nl pl pt-BR pt-PT ro ru sk sl sr sv sw ta te th tr uk vi zh-CN zh-TW
for(LOC, LOCALE_LIST) {
    locales.files += $$REPACK_DIR/qtwebengine_locales/$${LOC}.pak
}
resources.files = $$REPACK_DIR/qtwebengine_resources.pak \
    $$REPACK_DIR/qtwebengine_resources_100p.pak \
    $$REPACK_DIR/qtwebengine_resources_200p.pak \
    $$REPACK_DIR/qtwebengine_devtools_resources.pak

icu.files = $$OUT_PWD/$$getConfigDir()/icudtl.dat

!debug_and_release|!build_all|CONFIG(release, debug|release) {
    contains(QT_CONFIG, qt_framework) {
        locales.version = Versions
        locales.path = Resources/qtwebengine_locales
        resources.version = Versions
        resources.path = Resources
        icu.version = Versions
        icu.path = Resources
        # No files, this prepares the bundle Helpers symlink, process.pro will create the directories
        qtwebengineprocessplaceholder.version = Versions
        qtwebengineprocessplaceholder.path = Helpers
        QMAKE_BUNDLE_DATA += icu locales resources qtwebengineprocessplaceholder
    } else {
        locales.CONFIG += no_check_exist
        locales.path = $$[QT_INSTALL_TRANSLATIONS]/qtwebengine_locales
        resources.CONFIG += no_check_exist
        resources.path = $$[QT_INSTALL_DATA]/resources
        INSTALLS += locales resources

        !use?(system_icu) {
            icu.CONFIG += no_check_exist
            icu.path = $$[QT_INSTALL_DATA]/resources
            INSTALLS += icu
        }
    }

    !contains(QT_CONFIG, qt_framework):!force_independent {
        #
        # Copy essential files to the qtbase build directory for non-prefix builds
        #

        !use?(system_icu) {
            icudt2build.input = icu.files
            icudt2build.output = $$[QT_INSTALL_DATA/get]/resources/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}
            icudt2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
            icudt2build.name = COPY ${QMAKE_FILE_IN}
            icudt2build.CONFIG = no_link no_clean target_predeps
            QMAKE_EXTRA_COMPILERS += icudt2build
        }

        resources2build.input = resources.files
        resources2build.output = $$[QT_INSTALL_DATA/get]/resources/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}
        resources2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
        resources2build.name = COPY ${QMAKE_FILE_IN}
        resources2build.CONFIG = no_link no_clean target_predeps

        QMAKE_EXTRA_COMPILERS += resources2build

        locales2build.input = locales.files
        locales2build.output = $$[QT_INSTALL_DATA/get]/translations/qtwebengine_locales/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}
        locales2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
        locales2build.name = COPY ${QMAKE_FILE_IN}
        locales2build.CONFIG = no_link no_clean target_predeps

        QMAKE_EXTRA_COMPILERS += locales2build
    }
}

OTHER_FILES = \
    $$files(../3rdparty/chromium/*.h, true) \
    $$files(../3rdparty/chromium/*.cc, true) \
    $$files(../3rdparty/chromium/*.mm, true) \
    $$files(../3rdparty/chromium/*.py, true) \
    $$files(../3rdparty/chromium/*.gyp, true) \
    $$files(../3rdparty/chromium/*.gypi, true)
