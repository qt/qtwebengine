include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private

linking_pri = $$OUT_PWD/$$getConfigDir()/$${TARGET}.pri

!include($$linking_pri) {
    error("Could not find the linking information that gn should have generated.")
}

# Do not precompile any headers. We are only interested in the linker step.
PRECOMPILED_HEADER =

isEmpty(NINJA_OBJECTS): error("Missing object files from linking pri.")
isEmpty(NINJA_LFLAGS): error("Missing linker flags from linking pri")
isEmpty(NINJA_ARCHIVES): error("Missing archive files from linking pri")
isEmpty(NINJA_LIBS): error("Missing library files from linking pri")
NINJA_OBJECTS = $$eval($$list($$NINJA_OBJECTS))
# Do manual response file linking for macOS and Linux

RSP_OBJECT_FILE = $$OUT_PWD/$$getConfigDir()/$${TARGET}_o.rsp
for(object, NINJA_OBJECTS): RSP_O_CONTENT += $$object
write_file($$RSP_OBJECT_FILE, RSP_O_CONTENT)
RSP_ARCHIVE_FILE = $$OUT_PWD/$$getConfigDir()/$${TARGET}_a.rsp
for(archive, NINJA_ARCHIVES): RSP_A_CONTENT += $$archive
write_file($$RSP_ARCHIVE_FILE, RSP_A_CONTENT)

if(macos|ios) {
    !static {
        QMAKE_LFLAGS += -Wl,-filelist,$$shell_quote($${RSP_OBJECT_FILE})
        QMAKE_LFLAGS += @$${RSP_ARCHIVE_FILE}
    } else {
        OBJECTS += $$NINJA_OBJECTS
        LIBS_PRIVATE += $${NINJA_ARCHIVES}
    }
}

linux {
    !static {
        QMAKE_LFLAGS += @$${RSP_OBJECT_FILE}
        QMAKE_LFLAGS += -Wl,--start-group @$${RSP_ARCHIVE_FILE} -Wl,--end-group
    } else {
        OBJECTS += $$NINJA_OBJECTS
        LIBS_PRIVATE += -Wl,--start-group $${NINJA_ARCHIVES} -Wl,--end-group
    }
}

win32 {
    !static {
        QMAKE_LFLAGS += @$${RSP_OBJECT_FILE}
        QMAKE_LFLAGS += @$${RSP_ARCHIVE_FILE}
    } else {
        OBJECTS += $$NINJA_OBJECTS
        LIBS_PRIVATE += $${NINJA_ARCHIVES}
    }
}

LIBS_PRIVATE += $$NINJA_LIB_DIRS $$NINJA_LIBS
# GN's LFLAGS doesn't always work across all the Linux configurations we support.
# The Windows and macOS ones from GN does provide a few useful flags however

unix:qtConfig(webengine-noexecstack): \
    QMAKE_LFLAGS += -Wl,-z,noexecstack
linux {
    # add chromium flags
    for(flag, NINJA_LFLAGS) {
        # filter out some flags
        !contains(flag, .*noexecstack$): \
        !contains(flag, .*as-needed$): \
        !contains(flag, ^-B.*): \
        !contains(flag, ^-fuse-ld.*): \
        QMAKE_LFLAGS += $$flag
    }
} else {
    QMAKE_LFLAGS += $$NINJA_LFLAGS
}

POST_TARGETDEPS += $$NINJA_TARGETDEPS
