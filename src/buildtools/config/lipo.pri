for(arch, QT_ARCHS) {

    linking_pri = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}.pri

    !include($$linking_pri) {
        message("Could not find the linking information that gn should have generated.")
    }

    # Do not precompile any headers. We are only interested in the linker step.
    PRECOMPILED_HEADER =

    isEmpty(NINJA_OBJECTS): error("Missing object files from linking pri.")
    isEmpty(NINJA_LFLAGS): error("Missing linker flags from linking pri")
    isEmpty(NINJA_ARCHIVES): error("Missing archive files from linking pri")
    isEmpty(NINJA_LIBS): error("Missing library files from linking pri")
    NINJA_OBJECTS = $$eval($$list($$NINJA_OBJECTS))
    # Do manual response files


    RSP_OBJECT_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_objects.rsp
    for(object, NINJA_OBJECTS): RSP_OBJECTS_CONTENT += $$object
    write_file($$RSP_OBJECT_FILE, RSP_OBJECTS_CONTENT)
    RSP_ARCHIVE_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_archives.rsp
    for(archive, NINJA_ARCHIVES): RSP_ARCHIVES_CONTENT += $$archive
    write_file($$RSP_ARCHIVE_FILE, RSP_ARCHIVES_CONTENT)
    RSP_LIBS_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_libs.rsp
    for(lib, NINJA_LIBS): RSP_LIBS_CONTENT += $$lib
    write_file($$RSP_LIBS_FILE, RSP_LIBS_CONTENT)

    unset(RSP_OBJECTS_CONTENT)
    unset(RSP_ARCHIVES_CONTENT)
    unset(RSP_LIBS_CONTENT)
    unset(NINJA_OBJECTS)
    unset(NINJA_LFLAGS)
    unset(NINJA_ARCHIVES)
    unset(NINJA_LIBS)
}

LIPO_OUT_FILE = $$OUT_PWD/$$getConfigDir()/$${TARGET}.a
INPUT_FILE = .
lipo.name = lipo
lipo.output = $$LIPO_OUT_FILE
lipo.input = INPUT_FILE
lipo.CONFIG += target_predeps no_link
lipo.commands = lipo -create -output $$LIPO_OUT_FILE
QMAKE_EXTRA_COMPILERS += lipo

include($$QTWEBENGINE_ROOT/src/buildtools/config/mac_osx.pri)

for(arch, QT_ARCHS) {

    RSP_OBJECT_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_objects.rsp
    OBJECT_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_objects.o
    RSP_ARCHIVE_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_archives.rsp
    ARCHIVE_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}_archives.o
    OUT_FILE = $$OUT_PWD/$$arch/$$getConfigDir()/$${TARGET}.a

    intermediate_archive_$${arch}.name = build_intermediate_archive_$${arch}
    intermediate_archive_$${arch}.output= $$OUT_FILE
    intermediate_archive_$${arch}.input = INPUT_FILE
    intermediate_archive_$${arch}.depends = $$RSP_OBJECT_FILE $$RSP_ARCHIVE_FILE $$NINJA_TARGETDEPS
    intermediate_archive_$${arch}.CONFIG += target_predeps no_link
    intermediate_archive_$${arch}.commands = \
       clang++ -r -nostdlib -arch $$arch \
       -mmacosx-version-min=$${QMAKE_MACOSX_DEPLOYMENT_TARGET} \
       -o $$OBJECT_FILE \
       -Wl,-keep_private_externs \
       @$$RSP_OBJECT_FILE ;\
       $$QMAKE_CC -r -nostdlib -arch $$arch \
       -mmacosx-version-min=$${QMAKE_MACOSX_DEPLOYMENT_TARGET} \
       -o $$ARCHIVE_FILE \
       -Wl,-keep_private_externs \
       -Wl,-all_load \
       @$$RSP_ARCHIVE_FILE ;\
       ar -crs $$OUT_FILE $$OBJECT_FILE $$ARCHIVE_FILE
    lipo.depends += $$OUT_FILE
    lipo.commands += $$OUT_FILE
    QMAKE_EXTRA_COMPILERS += intermediate_archive_$$arch

    unset(RSP_OBJECT_FILE)
    unset(OBJECT_FILE)
    unset(RSP_ARCHVIE_FILE)
    unset(OUT_FILE)
}

