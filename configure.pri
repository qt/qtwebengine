include(src/buildtools/config/support.pri)
include(src/buildtools/config/functions.pri)

# this must be done outside any function
QTWEBENGINE_SOURCE_TREE = $$PWD

equals(QMAKE_HOST.os, Windows): EXE_SUFFIX = .exe

defineTest(isPythonVersionSupported) {
    python = $$system_quote($$system_path($$1))
    python_version = $$system('$$python -c "import sys; print(sys.version_info[0:3])"')
    python_version ~= s/[()]//g
    python_version = $$split(python_version, ',')
    python_major_version = $$first(python_version)
    greaterThan(python_major_version, 2) {
        qtLog("Python version 3 is not supported by Chromium.")
        return(false)
    }
    python_minor_version = $$member(python_version, 1)
    python_patch_version = $$member(python_version, 2)
    greaterThan(python_major_version, 1): greaterThan(python_minor_version, 6): greaterThan(python_patch_version, 4): return(true)
    qtLog("Unsupported python version: $${python_major_version}.$${python_minor_version}.$${python_patch_version}.")
    return(false)
}

defineTest(qtConfTest_detectJumboBuild) {
    mergeLimit = $$eval(config.input.merge_limit)
    mergeLimit = $$find(mergeLimit, "\\d")
    isEmpty(mergeLimit) {
       win32: mergeLimit = 0
       else: mergeLimit = 8
    }
    qtLog("Setting jumbo build merge batch limit to $${mergeLimit}.")
    $${1}.merge_limit = $$mergeLimit
    export($${1}.merge_limit)
    $${1}.cache += merge_limit
    export($${1}.cache)

    return(true)
}

defineTest(qtConfReport_skipBuildWarning) {
    $${1}()
    !isEmpty(skipBuildReason):qtConfAddWarning($${skipBuildReason})
}

defineTest(qtConfReport_jumboBuild) {
    mergeLimit = $$eval(cache.webengine-jumbo-build.merge_limit)
    isEmpty(mergeLimit)|!greaterThan(mergeLimit,0) {
       mergeLimit = "no"
    }
    qtConfReportPadded($${1}, $$mergeLimit)
}

defineTest(qtConfTest_detectPython2) {
    python = $$qtConfFindInPath("python2$$EXE_SUFFIX")
    isEmpty(python) {
        qtLog("'python2$$EXE_SUFFIX' not found in PATH. Checking for 'python$$EXE_SUFFIX'.")
        python = $$qtConfFindInPath("python$$EXE_SUFFIX")
    }
    isEmpty(python) {
        qtLog("'python$$EXE_SUFFIX' not found in PATH. Giving up.")
        return(false)
    }
    !isPythonVersionSupported($$python) {
        qtLog("A suitable Python 2 executable could not be located.")
        return(false)
    }

    # Make tests.python2.location available in configure.json.
    $${1}.location = $$clean_path($$python)
    export($${1}.location)
    $${1}.cache += location
    export($${1}.cache)

    return(true)
}

defineReplace(qtConfFindGnuTool) {
    equals(QMAKE_HOST.os, Windows) {
        gnuwin32bindir = $$absolute_path($$QTWEBENGINE_SOURCE_TREE/../gnuwin32/bin)
        gnuwin32toolpath = "$$gnuwin32bindir/$${1}"
        exists($$gnuwin32toolpath): \
            return($$gnuwin32toolpath)
    }
    return($$qtConfFindInPath($$1))
}

defineTest(qtConfTest_detectGperf) {
    gperf = $$qtConfFindGnuTool("gperf$$EXE_SUFFIX")
    isEmpty(gperf) {
        qtLog("Required gperf could not be found.")
        return(false)
    }
    qtLog("Found gperf from path: $$gperf")
    return(true)
}

defineTest(qtConfTest_detectBison) {
    bison = $$qtConfFindGnuTool("bison$$EXE_SUFFIX")
    isEmpty(bison) {
        qtLog("Required bison could not be found.")
        return(false)
    }
    qtLog("Found bison from path: $$bison")
    return(true)
}

defineTest(qtwebengine_platformError) {
    platformError = $$1
    export(platformError)
}

defineTest(qtConfTest_detectPlatform) {
    QT_FOR_CONFIG += gui-private

    linux:qtwebengine_isLinuxPlatformSupported() {
        $${1}.platform = "linux"
    }
    win32:qtwebengine_isWindowsPlatformSupported() {
        $${1}.platform = "windows"
    }
    macos:qtwebengine_isMacOsPlatformSupported() {
        $${1}.platform = "macos"
    }
    ios:qtwebengine_isMacOsPlatformSupported() {
        $${1}.platform = "ios"
    }

    !isEmpty(platformError) {
        qtLog("Platform not supported.")
        $${1}.platformSupport = $$platformError
        export($${1}.platformSupport)
        $${1}.cache += platformSupport
        export($${1}.cache)
        return(false)
    }
    export($${1}.platformSupport)
    return(true)
}

defineTest(qtConfTest_detectArch) {
    contains(QT_ARCH, "i386")|contains(QT_ARCH, "x86_64"): return(true)
    contains(QT_ARCH, "arm")|contains(QT_ARCH, "arm64"): return(true)
    contains(QT_ARCH, "mips"): return(true)
    qtLog("Architecture not supported.")
    return(false)
}

defineTest(qtConfTest_detectFlex) {
    flex = $$qtConfFindGnuTool("flex$$EXE_SUFFIX")
    isEmpty(flex) {
        qtLog("Required flex could not be found.")
        return(false)
    }
    qtLog("Found flex from path: $$flex")
    return(true)
}

defineTest(qtConfTest_detectNinja) {
    ninja = $$qtConfFindInPath("ninja$$EXE_SUFFIX")
    !isEmpty(ninja) {
        qtLog("Found ninja from path: $$ninja")
        qtRunLoggedCommand("$$ninja --version", version)|return(false)
        contains(version, "1\.([7-9]|1[0-9])\..*"): return(true)
        qtLog("Ninja version too old")
    }
    qtLog("Building own ninja")
    return(false)
}

defineTest(qtConfTest_detectProtoc) {
    protoc = $$qtConfFindInPath("protoc")
    isEmpty(protoc) {
        qtLog("Optional protoc could not be found.")
        return(false)
    }
    qtLog("Found protoc from path: $$protoc")
    return(true)
}

defineTest(qtConfTest_detectGn) {
    gn = $$qtConfFindInPath("gn$$EXE_SUFFIX")
    !isEmpty(gn) {
        qtRunLoggedCommand("$$gn --version", version)|return(false)
        #accept all for now
        contains(version, ".*"): return(true)
        qtLog("Gn version too old")
    }
    qtLog("Building own gn")
    return(false)
}

defineTest(qtConfTest_detectNodeJS) {
    nodejs = $$qtConfFindInPath("nodejs$$EXE_SUFFIX")
    isEmpty(nodejs) {
        qtLog("'nodejs$$EXE_SUFFIX' not found in PATH. Checking for 'node$$EXE_SUFFIX'.")
        nodejs = $$qtConfFindInPath("node$$EXE_SUFFIX")
        isEmpty(nodejs) {
            qtLog("'node$$EXE_SUFFIX' not found in PATH. Giving up.")
            return(false)
        }
    }
    return(true)
}

defineTest(qtConfTest_detectEmbedded) {
    lessThan(QT_MINOR_VERSION, 9) {
        cross_compile: return(true)
        return(false)
    }
    $$qtConfEvaluate("features.cross_compile"): return(true)
    return(false)
}

defineTest(qtConfTest_detectHostPkgConfig) {
   PKG_CONFIG = $$qtConfPkgConfig(true)
   isEmpty(PKG_CONFIG) {
       qtLog("Could not find host pkg-config")
       return(false)
   }
   qtLog("Found host pkg-config: $$PKG_CONFIG")
   $${1}.path = $$PKG_CONFIG
   export($${1}.path)
   $${1}.cache += path
   export($${1}.cache)
   return(true)
}

defineTest(qtConfTest_isSanitizerSupported) {
  sanitizer_combo_supported = true

  sanitize_address {
    asan_supported = false
    linux-clang-libc++:isSanitizerSupportedOnLinux() {
      asan_supported = true
    } else:macos:isSanitizerSupportedOnMacOS() {
      asan_supported = true
    }
    !$$asan_supported {
      sanitizer_combo_supported = false
      qtLog("An address sanitizer-enabled Qt WebEngine build can only be built on Linux or macOS using Clang and libc++.")
    }
  }

  sanitize_memory {
    sanitizer_combo_supported = false
    qtLog("A memory sanitizer-enabled Qt WebEngine build is not supported.")
  }

  sanitize_undefined {
    ubsan_supported = false
    CONFIG(release, debug|release):!debug_and_release {
      linux-clang-libc++:isSanitizerSupportedOnLinux() {
        ubsan_supported = true
      } else:macos:isSanitizerSupportedOnMacOS() {
        ubsan_supported = true
      }
    }
    !$$ubsan_supported {
      sanitizer_combo_supported = false
      qtLog("An undefined behavior sanitizer-enabled Qt WebEngine build can only be built on Linux or macOS using Clang and libc++ in release mode.")
    }
  }

  sanitize_thread {
    tsan_supported = false
    linux-clang-libc++:isSanitizerSupportedOnLinux() {
      tsan_supported = true
    }
    !$$tsan_supported {
      sanitizer_combo_supported = false
      qtLog("A thread sanitizer-enabled Qt WebEngine build can only be built on Linux using Clang and libc++.")
    }
  }

  $$sanitizer_combo_supported: return(true)
  return(false)
}

defineTest(isSanitizerSupportedOnLinux) {
  isSanitizerLinuxClangVersionSupported(): return(true)
  return(false)
}

defineTest(isSanitizerSupportedOnMacOS) {
  isEmpty(QMAKE_APPLE_CLANG_MAJOR_VERSION) {
    QTWEBENGINE_CLANG_IS_APPLE = false
  } else {
    QTWEBENGINE_CLANG_IS_APPLE = true
  }

  $$QTWEBENGINE_CLANG_IS_APPLE:isSanitizerMacOSAppleClangVersionSupported(): return(true)
  else:isSanitizerMacOSClangVersionSupported(): return(true)
  return(false)
}

defineTest(isSanitizerMacOSAppleClangVersionSupported) {
  # Clang sanitizer suppression attributes work from Apple Clang version 7.3.0+.
  greaterThan(QMAKE_APPLE_CLANG_MAJOR_VERSION, 7): return(true)
  greaterThan(QMAKE_APPLE_CLANG_MINOR_VERSION, 2): return(true)

  qtLog("Using Apple Clang version $${QMAKE_APPLE_CLANG_MAJOR_VERSION}.$${QMAKE_APPLE_CLANG_MINOR_VERSION}.$${QMAKE_APPLE_CLANG_PATCH_VERSION}, but at least Apple Clang version 7.3.0 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineTest(isSanitizerMacOSClangVersionSupported) {
  # Clang sanitizer suppression attributes work from non-apple Clang version 3.7+.
  greaterThan(QMAKE_CLANG_MAJOR_VERSION, 3): return(true)
  greaterThan(QMAKE_CLANG_MINOR_VERSION, 6): return(true)

  qtLog("Using Clang version $${QMAKE_CLANG_MAJOR_VERSION}.$${QMAKE_CLANG_MINOR_VERSION}, but at least Clang version 3.7 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineTest(isSanitizerLinuxClangVersionSupported) {
  # Clang sanitizer suppression attributes work from Clang version 3.7+.
  greaterThan(QMAKE_CLANG_MAJOR_VERSION, 3): return(true)
  greaterThan(QMAKE_CLANG_MINOR_VERSION, 6): return(true)

  qtLog("Using Clang version $${QMAKE_CLANG_MAJOR_VERSION}.$${QMAKE_CLANG_MINOR_VERSION}, but at least Clang version 3.7 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineReplace(qtConfFunc_isTestsInBuildParts) {
    contains(QT_BUILD_PARTS, tests): return(true)
    return(false)
}

defineReplace(webEngineGetMacOSVersion) {
    value = $$system("sw_vers -productVersion 2>/dev/null")
    return($$value)
}

defineReplace(webEngineGetMacOSSDKVersion) {
    value = $$system("/usr/bin/xcodebuild -sdk $$QMAKE_MAC_SDK -version ProductVersion 2>/dev/null")
    return($$value)
}

defineReplace(webEngineGetMacOSClangVerboseVersion) {
    output = $$system("$$QMAKE_CXX --version 2>/dev/null", lines)
    value = $$first(output)
    return($$value)
}

defineTest(qtConfReport_macosToolchainVersion) {
    arg = $$2
    contains(arg, "macosVersion"): report_message = $$webEngineGetMacOSVersion()
    contains(arg, "xcodeVersion"): report_message = "$$QMAKE_XCODE_VERSION"
    contains(arg, "clangVersion"): report_message = $$webEngineGetMacOSClangVerboseVersion()
    contains(arg, "sdkVersion"): report_message = $$webEngineGetMacOSSDKVersion()
    contains(arg, "deploymentTarget"): report_message = "$$QMAKE_MACOSX_DEPLOYMENT_TARGET"
    !isEmpty(report_message): qtConfReportPadded($$1, $$report_message)
}

defineTest(qtConfTest_isWindowsHostCompiler64) {
    win_host_arch = $$(VSCMD_ARG_HOST_ARCH)
    isEmpty(win_host_arch): return(true)
    contains(win_host_arch,"x64"): return(true)
    qtLog("Required 64-bit cross-building or native toolchain was not detected.")
    return(false)
}

# Fixme QTBUG-71772
defineTest(qtConfTest_hasThumbFlag) {
    FLAG = $$qtwebengine_extractCFlag("-mthumb")
    !isEmpty(FLAG): return(true)
    FLAG = $$qtwebengine_extractCFlag("-marm")
    !isEmpty(FLAG): return(false)

    MARCH = $$qtwebengine_extractCFlag("-march=.*")
    MARMV = $$replace(MARCH, "armv",)
    !isEmpty(MARMV) {
        MARMV = $$split(MARMV,)
        MARMV = $$member(MARMV, 0)
    }
    if (isEmpty(MARMV) | lessThan(MARMV, 7)): return(false)
    # no flag assume mthumb
    return(true)
}

defineTest(qtConfTest_detectSubmodule) {
    isEmpty(QTWEBENGINE_ROOT) {
        # topLevel build , add poor man's workaround
        QTWEBENGINE_ROOT=$$PWD/../../../qtwebengine
    }
    !exists($$QTWEBENGINE_ROOT/src/3rdparty/chromium):return(false)
    return(true)
}

defineTest(qtConfTest_detectNoWhitespace) {
    WSPC = $$find(OUT_PWD, \\s)
    !isEmpty(WSPC):return(false)
    return(true)
}

defineTest(qtwebengine_confCheckWebEngineCoreError) {
    QT_FOR_CONFIG += buildtools-private gui-private
    return($$qtwebengine_checkWebEngineCoreError())
}

defineTest(qtwebengine_confCheckPdfError) {
    QT_FOR_CONFIG += buildtools-private gui-private
    return($$qtwebengine_checkPdfError())
}

defineTest(qtwebengine_isLinuxPlatformSupported) {
    !gcc|intel_icc {
        qtwebengine_platformError("requires clang or GCC.")
        return(false)
    }
    gcc:!clang:!qtwebengine_isGCCVersionSupported(): return(false)
    gcc:!qtConfig(c++14) {
        qtwebengine_platformError("requires c++14 support.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isWindowsPlatformSupported) {
    winrt {
        qtwebengine_platformError("for WinRT is not supported.")
        return(false)
    }
    qtwebengine_isBuildingOnWin32() {
        qtwebengine_platformError("must be built on a 64-bit machine.")
        return(false)
    }
    !msvc|intel_icl {
        qtwebengine_platformError("requires MSVC or Clang (MSVC mode).")
        return(false)
    }
    !qtwebengine_isMinWinSDKVersion(10, 18362): {
        qtwebengine_platformError("requires a Windows SDK version 10.0.18362 or newer.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isMacOsPlatformSupported) {
    !qtwebengine_isMinXcodeVersion(10, 0, 0) {
        qtwebengine_platformError("requires at least version 10.0.0, but using Xcode version $${QMAKE_XCODE_VERSION}.")
        return(false)
    }
    !clang|intel_icc {
        qtwebengine_platformError("requires Clang.")
        return(false)
    }
    # We require macOS 10.13 (darwin version 17.0.0) or newer.
    darwin_major_version = $$section(QMAKE_HOST.version, ., 0, 0)
    lessThan(darwin_major_version, 17) {
        qtwebengine_platformError("requires macOS version 10.13 or newer.")
        return(false)
    }
    !qtwebengine_isMinOSXSDKVersion(10, 13): {
        qtwebengine_platformError("requires a macOS SDK version of 10.13 or newer. Current version is $${WEBENGINE_OSX_SDK_PRODUCT_VERSION}.")
        return(false)
    }
    return(true)
}

defineTest(qtwebengine_isGCCVersionSupported) {
  # Keep in sync with src/webengine/doc/src/qtwebengine-platform-notes.qdoc
  greaterThan(QMAKE_GCC_MAJOR_VERSION, 4):return(true)

  qtwebengine_platformError("requires at least gcc version 5, but using gcc version $${QMAKE_GCC_MAJOR_VERSION}.$${QMAKE_GCC_MINOR_VERSION}.")
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
        qtwebengine_platformError("requires SDK product version, but could not resolve it for \'$$QMAKE_MAC_SDK\'.")
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
        qtwebengine_platformError("requires Windows SDK version, but could not detect it (\'WindowsSDKVersion\' environment variable is not set).")
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
