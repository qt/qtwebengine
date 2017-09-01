load(qt_parts)

isPlatformSupported() {
    !exists(src/3rdparty/chromium): \
        error("Submodule qtwebengine-chromium does not exist. Run 'git submodule update --init'.")
    WSPC = $$find(OUT_PWD, \\s)
    !isEmpty(WSPC): \
        error("QtWebEngine cannot be built in a path that contains whitespace characters.")
    load(configure)
    runConfigure()
}

!isEmpty(skipBuildReason) {
    SUBDIRS =
    log($${skipBuildReason}$${EOL})
    log(QtWebEngine will not be built.$${EOL})
}

QMAKE_DISTCLEAN += .qmake.cache

OTHER_FILES = \
    tools/buildscripts/* \
    tools/scripts/* \
    config.tests/khr/* \
    config.tests/libcap/* \
    config.tests/libvpx/* \
    config.tests/snappy/* \
    config.tests/re2/* \
    mkspecs/features/*
