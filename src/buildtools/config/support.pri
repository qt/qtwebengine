defineTest(qtwebengine_skipBuild) {
    skipBuildReason = $$1
    export(skipBuildReason)
}

# this should match webengine-core-support
defineReplace(qtwebengine_checkWebEngineCoreError) {
    !linux:!win32:!macos {
        qtwebengine_skipBuild("QtWebEngine can be built only on Linux, Windows or macOS.")
        return(false)
    }
    static {
        qtwebengine_skipBuild("Static builds of QtWebEngine are not supported.")
        return(false)
    }
    !qtwebengine_checkForGui(QtWebEngine):return(false)
    !qtwebengine_checkForSubmodule(QtWebEngine):return(false)
    !qtwebengine_checkForWhiteSpace(QtWebEngine):return(false)
    !qtwebengine_checkForPlatform(QtWebEngine):return(false)
    !qtwebengine_checkForArch(QtWebEngine):return(false)
    !qtwebengine_checkForGperf(QtWebEngine):return(false)
    !qtwebengine_checkForBison(QtWebEngine):return(false)
    !qtwebengine_checkForFlex(QtWebEngine):return(false)
    !qtwebengine_checkForPython2(QtWebengine):return(false)
    !qtwebengine_checkForSanitizer(QtWebEngine):return(false)
    linux:!qtwebengine_checkForPkgCfg(QtWebEngine):return(false)
    linux:!qtwebengine_checkForHostPkgCfg(QtWebEngine):return(false)
    linux:!qtwebengine_checkForGlibc(QtWebEngine):return(false)
    linux:!qtwebengine_checkForKhronos(QtWebEngine):return(false)
    linux:!qtwebengine_checkForPackage(QtWebEngine,nss):return(false)
    linux:!qtwebengine_checkForPackage(QtWebEngine,dbus):return(false)
    linux:!qtwebengine_checkForPackage(QtWebEngine,fontconfig):return(false)
    linux:!qtwebengine_checkForQpaXcb(QtWebEngine):return(false)
    win32:!qtwebengine_checkForCompiler64(QtWebEngine):return(false)
    win32:!qtwebengine_checkForWinVersion(QtWebEngine):return(false)
    return(true)
}

# this shuold match webengine-qtpdf-support
defineReplace(qtwebengine_checkPdfError) {
    !linux:!win32:!macos:!ios {
        qtwebengine_skipBuild("QtPdf can be built only on Linux, Windows, macOS or iOS.")
        return(false)
    }
    !qtwebengine_checkForGui(QtPdf):return(false)
    !qtwebengine_checkForSubmodule(QtPdf):return(false)
    !qtwebengine_checkForWhiteSpace(QtPdf):return(false)
    !qtwebengine_checkForPlatform(QtPdf):return(false)
    !qtwebengine_checkForArch(QtPdf):return(false)
    !qtwebengine_checkForGperf(QtPdf):return(false)
    !qtwebengine_checkForBison(QtPdf):return(false)
    !qtwebengine_checkForFlex(QtPdf):return(false)
    !qtwebengine_checkForPython2(QtPdf):return(false)
    !qtwebengine_checkForSanitizer(QtPdf):return(false)
    linux:!qtwebengine_checkForPkgCfg(QtPdf):return(false)
    linux:!qtwebengine_checkForHostPkgCfg(QtPdf):return(false)
    win32:!qtwebengine_checkForWinVersion(QtPdf):return(false)
    return(true)
}

defineTest(qtwebengine_checkForGui) {
    module = $$1
    !qtHaveModule(gui) {
        qtwebengine_skipBuild("$${module} requires QtGui.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForSubmodule) {
    module = $$1
    !qtConfig(webengine-submodule) {
        qtwebengine_skipBuild("$${module} required submodule qtwebengine-chromium does not exist. Run 'git submodule update --init'.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForWhiteSpace) {
    module = $$1
    !qtConfig(webengine-nowhitespace) {
        qtwebengine_skipBuild("$${module} cannot be built in a path that contains whitespace characters.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForPlatform) {
    module = $$1
    qtConfig(webengine-no-platform-support) {
        !isEmpty(platformError) {
            qtwebengine_skipBuild("$${module} $${platformError}")
            return(false)
        }
        !isEmpty(QTWEBENGINE_OUT_ROOT) {
            include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
            QT_FOR_CONFIG += buildtools-private
            !isEmpty(PLATFORM_ERROR) {
                qtwebengine_skipBuild("$${module} $${PLATFORM_ERROR}")
                return(false)
            }
        }
        qtwebengine_skipBuild("$${module} will not be built. Platform unsupported.")
        return(false):
    }
    return(true)
}

defineTest(qtwebengine_checkForArch) {
    module = $$1
    !qtConfig(webengine-arch-support) {
        qtwebengine_skipBuild("$${module} can only be built for x86, x86-64, ARM, Aarch64, and MIPSel architectures.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForGperf) {
    module = $$1
    !qtConfig(webengine-gperf) {
        qtwebengine_skipBuild("Tool gperf is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForBison) {
    module = $$1
    !qtConfig(webengine-bison) {
        qtwebengine_skipBuild("Tool bison is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForFlex) {
    module = $$1
    !qtConfig(webengine-flex) {
        qtwebengine_skipBuild("Tool flex is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForPython2) {
    module = $$1
    !qtConfig(webengine-python2) {
        qtwebengine_skipBuild("Python version 2 (2.7.5 or later) is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForSanitizer) {
    module = $$1
    sanitizer:!qtConfig(webengine-sanitizer) {
        qtwebengine_skipBuild("Chosen sanitizer configuration is not supported for $${module}." \
                              "Check config.log for details or use -feature-webengine-sanitizer to force build with the chosen sanitizer configuration.")
        return(false);
    }
    return(true)
}

defineTest(qtwebengine_checkForPkgCfg) {
    module = $$1
    !qtConfig(pkg-config) {
        qtwebengine_skipBuild("A pkg-config support is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForHostPkgCfg) {
    module = $$1
    !qtConfig(webengine-host-pkg-config) {
        qtwebengine_skipBuild("Host pkg-config is required to build $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForGlibc) {
    module = $$1
    !qtConfig(webengine-system-glibc) {
        qtwebengine_skipBuild("A suitable version >= 2.27 of libc required to build $${module} could not be found.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForKhronos) {
    module = $$1
    !qtConfig(webengine-system-khr) {
        qtwebengine_skipBuild("Khronos development headers required to build $${module} are missing (see mesa/libegl1-mesa-dev)")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForPackage) {
    module = $$1
    package = $$2
    !qtConfig(webengine-system-$$package) {
        qtwebengine_skipBuild("A suitable version of $$package required to build QtWebEngine could not be found.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForQpaXcb) {
    module = $$1
    qtConfig(pkg-config):qtConfig(xcb):!qtConfig(webengine-ozone-x11) {
        qtwebengine_skipBuild("Could not find all necessary libraries for qpa-xcb support in $${module}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForCompiler64) {
    module = $$1
    !qtConfig(webengine-win-compiler64) {
        qtwebengine_skipBuild("64-bit cross-building or native toolchain required to build $${module} could not be found.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_checkForWinVersion) {
    module = $$1
    !qtConfig(webengine-winversion) {
        qtwebengine_skipBuild("$${module} requires Visual Studio 2017 or higher.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_makeCheckWebEngineCoreError) {
    include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
    QT_FOR_CONFIG += buildtools-private gui-private
    return($$qtwebengine_checkWebEngineCoreError())
}

defineTest(qtwebengine_makeCheckPdfError) {
    include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
    QT_FOR_CONFIG += buildtools-private gui-private
    return($$qtwebengine_checkPdfError())
}
