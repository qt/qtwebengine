defineTest(qtwebengine_skipBuild) {
    isEmpty(skipBuildReason): skipBuildReason = $$1
    else: skipBuildReason = "$$skipBuildReason $${EOL}$$1"
    export(skipBuildReason)
}

defineReplace(qtwebengine_checkError) {

    static {
       qtwebengine_skipBuild("Static builds of QtWebEngine are not supported.")
       return(false)
    }

    !qtHaveModule(gui) {
        qtwebengine_skipBuild("QtWebEngine requires QtGui.")
        return(false)
    }

    !qtConfig(webengine-submodule) {
        qtwebengine_skipBuild("QtWebEngine required submodule qtwebengine-chromium does not exist. Run 'git submodule update --init'.")
        return(false)
    }

    !qtConfig(webengine-nowhitespace) {
        qtwebengine_skipBuild("QtWebEngine cannot be built in a path that contains whitespace characters.")
        return(false)
    }

    qtConfig(webengine-no-platform-support) {
        !isEmpty(platformError) {
            qtwebengine_skipBuild($$platformError)
            return(false)
        }
        !isEmpty(QTWEBENGINE_OUT_ROOT) {
            include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
            QT_FOR_CONFIG += buildtools-private
            qtwebengine_skipBuild($$PLATFORM_ERROR)
            return(false)
        }
        qtwebengine_skipBuild("QtWebEngine will not be built. Platform unsupported.") # re-run of configure when topLevel build
        return(false):
    }

    !qtConfig(webengine-arch-support) {
        qtwebengine_skipBuild("QtWebEngine can only be built for x86, x86-64, ARM, Aarch64, and MIPSel architectures.")
        return(false)
    }

    !qtConfig(webengine-gperf) {
        qtwebengine_skipBuild("Tool gperf is required to build QtWebEngine.")
        return(false)
    }

    !qtConfig(webengine-bison) {
        qtwebengine_skipBuild("Tool bison is required to build QtWebEngine.")
        return(false)
    }

    !qtConfig(webengine-flex) {
        qtwebengine_skipBuild("Tool flex is required to build QtWebEngine.")
        return(false)
    }

    !qtConfig(webengine-python2) {
        qtwebengine_skipBuild("Python version 2 (2.7.5 or later) is required to build QtWebEngine.")
        return(false)
    }

    linux:!qtwebengine_checkErrorForLinux():return(false)
    win:!qtwebengine_checkErrorForWindows():return(false)

    sanitizer: !qtConfig(webengine-sanitizer) {
        qtwebengine_skipBuild("Chosen sanitizer configuration is not supported for QtWebEngine. Check config.log for details or use -feature-webengine-sanitizer to force build with the chosen sanitizer configuration.")
        return(false);
    }

    return(true)
}

defineTest(qtwebengine_checkErrorForLinux) {

    !qtConfig(pkg-config) {
        qtwebengine_skipBuild("A pkg-config support is required to build QtWebEngine.")
        return(false)
    }

    !qtConfig(webengine-host-pkg-config) {
        qtwebengine_skipBuild("Host pkg-config is required to build QtWebEngine.")
        return(false)
    }

    !qtConfig(webengine-system-glibc) {
        qtwebengine_skipBuild("A suitable version >= 2.27 of libc required to build QtWebEngine could not be found.")
        return(false)
    }

    !qtConfig(webengine-system-khr) {
        qtwebengine_skipBuild("Khronos development headers required to build QtWebEngine are missing (see mesa/libegl1-mesa-dev)")
        return(false)
    }

    for(package, $$list("nss dbus fontconfig")) {
        !qtConfig(webengine-system-$$package) {
            qtwebengine_skipBuild("A suitable version of $$package required to build QtWebEngine could not be found.")
            return(false)
        }
    }

    qtConfig(pkg-config):qtConfig(xcb):!qtConfig(webengine-ozone-x11) {
        qtwebengine_skipBuild("Could not find all necessary libraries for qpa-xcb support in QtWebEngine.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkErrorForWindows) {
    !qtConfig(webengine-win-compiler64) {
        qtwebengine_skipBuild("64-bit cross-building or native toolchain required to build QtWebEngine could not be found.")
        return(false)
    }

    !qtConfig(webengine-winversion) {
        qtwebengine_skipBuild("QtWebEngine needs Visual Studio 2017 or higher.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_makeCheckError) {
    include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
    QT_FOR_CONFIG += buildtools-private gui-private
    return($$qtwebengine_checkError())
}
