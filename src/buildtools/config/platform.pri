defineTest(qtwebengine_isLinuxPlatformSupported) {
    !gcc|intel_icc {
        qtwebengine_skipBuild("Qt WebEngine on Linux requires clang or GCC.")
        return(false)
    }
    gcc:!clang:!qtwebengine_isGCCVersionSupported(): return(false)
    gcc:!qtConfig(c++14) {
        qtwebengine_skipBuild("C++14 support is required in order to build chromium.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isWindowsPlatformSupported) {
    winrt {
        qtwebengine_skipBuild("WinRT is not supported.")
        return(false)
    }
    qtwebengine_isBuildingOnWin32() {
        qtwebengine_skipBuild("Qt WebEngine on Windows must be built on a 64-bit machine.")
        return(false)
    }
    !msvc|intel_icl {
        qtwebengine_skipBuild("Qt WebEngine on Windows requires MSVC or Clang (MSVC mode).")
        return(false)
    }
    !qtwebengine_isMinWinSDKVersion(10, 17763): {
        qtwebengine_skipBuild("Qt WebEngine on Windows requires a Windows SDK version 10.0.17763 or newer.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isMacOsPlatformSupported) {
    # FIXME: Try to get it back down to 8.2 for building on OS X 10.11
    !qtwebengine_isMinXcodeVersion(8, 3, 3) {
        qtwebengine_skipBuild("Using Xcode version $$QMAKE_XCODE_VERSION, but at least version 8.3.3 is required to build Qt WebEngine.")
        return(false)
    }
    !clang|intel_icc {
        qtwebengine_skipBuild("Qt WebEngine on macOS requires Clang.")
        return(false)
    }
    # We require macOS 10.12 (darwin version 16.0.0) or newer.
    darwin_major_version = $$section(QMAKE_HOST.version, ., 0, 0)
    lessThan(darwin_major_version, 16) {
        qtwebengine_skipBuild("Building Qt WebEngine requires macOS version 10.12 or newer.")
        return(false)
    }
    !qtwebengine_isMinOSXSDKVersion(10, 12): {
        qtwebengine_skipBuild("Building Qt WebEngine requires a macOS SDK version of 10.12 or newer. Current version is $${WEBENGINE_OSX_SDK_PRODUCT_VERSION}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isPlatformSupported) {
    QT_FOR_CONFIG += gui-private
    !linux:!win32:!macos {
        qtwebengine_skipBuild("Unknown platform. Qt WebEngine only supports Linux, Windows, and macOS.")
        return(false)
    }
    linux:qtwebengine_isLinuxPlatformSupported(): return(true)
    win32:qtwebengine_isWindowsPlatformSupported(): return(true)
    macos:qtwebengine_isMacOsPlatformSupported(): return(true)
    return(false)
}

defineTest(qtwebengine_isArchSupported) {
    contains(QT_ARCH, "i386")|contains(QT_ARCH, "x86_64"): return(true)
    contains(QT_ARCH, "arm")|contains(QT_ARCH, "arm64"): return(true)
    contains(QT_ARCH, "mips"): return(true)
#     contains(QT_ARCH, "mips64"): return(true)

    qtwebengine_skipBuild("QtWebEngine can only be built for x86, x86-64, ARM, Aarch64, and MIPSel architectures.")
    return(false)
}

defineTest(qtwebengine_isGCCVersionSupported) {
  # Keep in sync with src/webengine/doc/src/qtwebengine-platform-notes.qdoc
  greaterThan(QMAKE_GCC_MAJOR_VERSION, 4):return(true)

  qtwebengine_skipBuild("Using gcc version "$$QMAKE_GCC_MAJOR_VERSION"."$$QMAKE_GCC_MINOR_VERSION", but at least gcc version 5 is required to build Qt WebEngine.")
  return(false)
}

defineTest(qtwebengine_isBuildingOnWin32) {
    # The check below is ugly, but necessary, as it seems to be the only reliable way to detect if the host
    # architecture is 32 bit. QMAKE_HOST.arch does not work as it returns the architecture that the toolchain
    # is building for, not the system's actual architecture.
    PROGRAM_FILES_X86 = $$(ProgramW6432)
    isEmpty(PROGRAM_FILES_X86): return(true)
    return(false)
}

defineTest(qtwebengine_isMinOSXSDKVersion) {
    requested_major = $$1
    requested_minor = $$2
    requested_patch = $$3
    isEmpty(requested_patch): requested_patch = 0
    WEBENGINE_OSX_SDK_PRODUCT_VERSION = $$system("/usr/bin/xcodebuild -sdk $$QMAKE_MAC_SDK -version ProductVersion 2>/dev/null")
    export(WEBENGINE_OSX_SDK_PRODUCT_VERSION)
    isEmpty(WEBENGINE_OSX_SDK_PRODUCT_VERSION) {
        qtwebengine_skipBuild("Could not resolve SDK product version for \'$$QMAKE_MAC_SDK\'.")
        return(false)
    }
    major_version = $$section(WEBENGINE_OSX_SDK_PRODUCT_VERSION, ., 0, 0)
    minor_version = $$section(WEBENGINE_OSX_SDK_PRODUCT_VERSION, ., 1, 1)
    patch_version = $$section(WEBENGINE_OSX_SDK_PRODUCT_VERSION, ., 2, 2)
    isEmpty(patch_version): patch_version = 0

    greaterThan(major_version, $$requested_major):return(true)
    equals(major_version, $$requested_major):greaterThan(minor_version, $$requested_minor):return(true)
    equals(major_version, $$requested_major):equals(minor_version, $$requested_minor):!lessThan(patch_version, $$requested_patch):return(true)

    return(false)
}

defineTest(qtwebengine_isMinXcodeVersion) {
    requested_major = $$1
    requested_minor = $$2
    requested_patch = $$3
    isEmpty(requested_minor): requested_minor = 0
    isEmpty(requested_patch): requested_patch = 0
    target_var = QMAKE_XCODE_VERSION
    major_version = $$section($$target_var, ., 0, 0)
    minor_version = $$section($$target_var, ., 1, 1)
    patch_version = $$section($$target_var, ., 2, 2)
    isEmpty(minor_version): minor_version = 0
    isEmpty(patch_version): patch_version = 0

    greaterThan(major_version, $$requested_major):return(true)
    equals(major_version, $$requested_major):greaterThan(minor_version, $$requested_minor):return(true)
    equals(major_version, $$requested_major):equals(minor_version, $$requested_minor):!lessThan(patch_version, $$requested_patch):return(true)

    return(false)
}

defineTest(qtwebengine_isMinWinSDKVersion) {
    requested_major = $$1
    requested_minor = $$2
    WIN_SDK_VERSION = $$(WindowsSDKVersion)

    isEmpty(WIN_SDK_VERSION)|equals(WIN_SDK_VERSION, "\\") {
        qtwebengine_skipBuild("Could not detect Windows SDK version (\'WindowsSDKVersion\' environment variable is not set).")
        return(false)
    }

    # major.0.minor
    major_version = $$section(WIN_SDK_VERSION, ., 0, 0)
    minor_version = $$section(WIN_SDK_VERSION, ., 2, 2)

    greaterThan(major_version, $$requested_major):return(true)
    equals(major_version, $$requested_major):greaterThan(minor_version, $$requested_minor):return(true)
    equals(major_version, $$requested_major):equals(minor_version, $$requested_minor)::return(true)

    return(false)
}

defineTest(qtwebengine_skipBuild) {
    isEmpty(skipBuildReason): skipBuildReason = $$1
    else: skipBuildReason = "$$skipBuildReason $${EOL}$$1"
    export(skipBuildReason)
}
