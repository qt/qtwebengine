# This .pro file serves a dual purpose:
# 1) invoking gyp through the gyp_qtwebengine script, which in turn makes use of the generated gypi include files
# 2) produce a Makefile that will run ninja, and take care of actually building everything.

TEMPLATE = aux

GYP_ARGS = "-D qt_cross_compile=0"
cross_compile {
    GYP_ARGS = "-D qt_cross_compile=1 -D os_posix=1"
    TOOLCHAIN_SYSROOT = $$[QT_SYSROOT]

    linux-android {
        CC = $$which($$QMAKE_CC)
        ANDROID_TOOLCHAIN = $$dirname(CC)
        TOOLCHAIN_SYSROOT = $$ANDROID_BUILD_TOP

        GYP_ARGS += "-D android_src=\"$${ANDROID_BUILD_TOP}\" -D android_toolchain=\"$${ANDROID_TOOLCHAIN}\"" \
                    "-D android_ndk_root=\"$${ANDROID_BUILD_TOP}\" -D android_product_out=\"$${ANDROID_PRODUCT_OUT}\""
    }

    !isEmpty(TOOLCHAIN_SYSROOT): GYP_ARGS += "-D sysroot=\"$${TOOLCHAIN_SYSROOT}\""

    contains(QT_ARCH, "arm") {
        GYP_ARGS += "-D target_arch=arm"

        # Extract ARM specific compiler options that we have to pass to gyp,
        # but let gyp figure out a default if an option is not present.
        MARCH = $$extractCFlag("-march=.*")
        !isEmpty(MARCH): GYP_ARGS += "-D arm_arch=\"$$MARCH\""

        MFPU = $$extractCFlag("-mfpu=.*")
        !isEmpty(MFPU) {
            GYP_ARGS += "-D arm_fpu=\"$$MFPU\""
            contains(MFPU, "neon"): GYP_ARGS += "-D arm_neon=1"
        }

        MTUNE = $$extractCFlag("-mtune=.*")
        !isEmpty(MTUNE): GYP_ARGS += "-D arm_tune=\"$$MTUNE\""

        MFLOAT = $$extractCFlag("-mfloat-abi=.*")
        !isEmpty(MFLOAT): GYP_ARGS += "-D arm_float_abi=\"$$MFLOAT\""

        MARMV = $$replace(MARCH, "armv",)
        !isEmpty(MARMV) {
            MARMV = $$split(MARMV,)
            MARMV = $$member(MARMV, 0)
            GYP_ARGS += "-D arm_version=\"$$MARMV\""
        }

        contains(QMAKE_CFLAGS, "-mthumb"): GYP_ARGS += "-D arm_thumb=1"
    }

    # Needed for v8, see chromium/v8/build/toolchain.gypi
    GYP_ARGS += "-D CXX=\"$$which($$QMAKE_CXX)\""
}

contains(WEBENGINE_CONFIG, proprietary_codecs): GYP_ARGS += "-Dproprietary_codecs=1 -Dffmpeg_branding=Chrome -Duse_system_ffmpeg=0"

!build_pass {
  message(Running gyp_qtwebengine $${GYP_ARGS}...)
  !system("python $$QTWEBENGINE_ROOT/tools/buildscripts/gyp_qtwebengine $${GYP_ARGS}"): error("-- running gyp_qtwebengine failed --")
}

ninja.target = invoke_ninja
ninja.commands = $$findOrBuildNinja() \$\(NINJAFLAGS\) -C $$getOutDir()/$$getConfigDir()
QMAKE_EXTRA_TARGETS += ninja

build_pass:build_all:default_target.target = all
else: default_target.target = first
default_target.depends = ninja

QMAKE_EXTRA_TARGETS += default_target
