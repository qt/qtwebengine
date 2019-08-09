load(qt_parts)
load(functions)
load(platform)

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

!isWebEngineCoreBuild():!isEmpty(skipBuildReason):!build_pass {
    log(QtWebEngine will not be built. $${skipBuildReason} $${EOL})
}
