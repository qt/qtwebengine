# This .pro file serves a dual purpose:
# 1) invoking gyp through the gyp_qtwebengine script, which in turn makes use of the generated gypi include files
# 2) produce a Makefile that will run ninja, and take care of actually building everything.

TEMPLATE = aux

GYP_ARGS = "-D qt_cross_compile=0"
cross_compile {
    GYP_ARGS = "-D qt_cross_compile=1 -D os_posix=1"
    TOOLCHAIN_SYSROOT = $$[QT_SYSROOT]

    android {
        CC = $$which($$QMAKE_CC)
        ANDROID_TOOLCHAIN = $$dirname(CC)
        TOOLCHAIN_SYSROOT = $$ANDROID_BUILD_TOP

        GYP_ARGS += "-D qt_os=\"android\" -D android_src=\"$${TOOLCHAIN_SYSROOT}\" -D android_toolchain=\"$${ANDROID_TOOLCHAIN}\"" \
                    "-D android_ndk_root=\"$${TOOLCHAIN_SYSROOT}\" -D android_product_out=\"$${ANDROID_PRODUCT_OUT}\""
    }

    !isEmpty(TOOLCHAIN_SYSROOT): GYP_ARGS += "-D sysroot=\"$${TOOLCHAIN_SYSROOT}\""

    contains(QT_ARCH, "arm") {
        GYP_ARGS += "-D target_arch=arm"

        # Extract ARM specific compiler options that we have to pass to gyp,
        # but let gyp figure out a default if an option is not present.
        MARCH = $$extractCFlag("-march=.*")
        !isEmpty(MARCH): GYP_ARGS += "-D arm_arch=\"$$MARCH\""

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

        MFPU = $$extractCFlag("-mfpu=.*")
        !isEmpty(MFPU) {
            # If the toolchain does not explicitly specify to use NEON instructions
            # we use arm_neon_optional for ARMv7 and newer and let chromium decide
            # about the mfpu option.
            contains(MFPU, "neon"): GYP_ARGS += "-D arm_fpu=\"$$MFPU\" -D arm_neon=1"
            else:!lessThan(MARMV, 7): GYP_ARGS += "-D arm_neon=0 -D arm_neon_optional=1"
            else: GYP_ARGS += "-D arm_fpu=\"$$MFPU\""
        }

        contains(QMAKE_CFLAGS, "-mthumb"): GYP_ARGS += "-D arm_thumb=1"
    }

    # Needed for v8, see chromium/v8/build/toolchain.gypi
    GYP_ARGS += "-D CXX=\"$$which($$QMAKE_CXX)\""
}

!build_pass {
  message(Running gyp_qtwebengine $${GYP_ARGS}...)
  !system("python $$QTWEBENGINE_ROOT/tools/buildscripts/gyp_qtwebengine $${GYP_ARGS}"): error("-- running gyp_qtwebengine failed --")
}

ninja.target = invoke_ninja
ninja.commands = $$findOrBuildNinja() \$\(NINJAFLAGS\) -C $$getOutDir()/$$getConfigDir()
QMAKE_EXTRA_TARGETS += ninja

handle_lib.target = handle_core_library
win32 {
# Gyp-Ninja creates Qt5WebEngineCore.dll and Qt5WebEngineCore.dll.lib in out\{Release|Debug}\ directory.
handle_lib.commands = "if exist $$system_path($$getOutDir()/$$getConfigDir()/lib/$${QTWEBENGINECORE_LIB_NAME}.*) del  $$system_path($$getOutDir()/$$getConfigDir()/lib/$${QTWEBENGINECORE_LIB_NAME}.*)$$escape_expand(\\n\\t)" \
                      "mklink /H $$system_path($$getOutDir()/$$getConfigDir()/lib/$${QTWEBENGINECORE_LIB_NAME}.dll) $$system_path($$getOutDir()/$$getConfigDir()/$${QTWEBENGINECORE_LIB_NAME}.dll)$$escape_expand(\\n\\t)" \
                      "mklink /H $$system_path($$getOutDir()/$$getConfigDir()/lib/$${QTWEBENGINECORE_LIB_NAME}.lib) $$system_path($$getOutDir()/$$getConfigDir()/$${QTWEBENGINECORE_LIB_NAME}.dll.lib)"

} else: macx {
# Gyp-Ninja creates Qt5WebEngineCore.so in out/{Release|Debug}/ directory.
handle_lib.commands = "ln -f $$getOutDir()/$$getConfigDir()/$${QTWEBENGINECORE_LIB_NAME}.so $$getOutDir()/$$getConfigDir()/lib/$${QTWEBENGINECORE_LIB_NAME}.so"
} else {
# Nothing to do on Linux
}
handle_lib.depends = ninja
QMAKE_EXTRA_TARGETS += handle_lib

build_pass:build_all:default_target.target = all
else: default_target.target = first
default_target.depends = handle_lib

QMAKE_EXTRA_TARGETS += default_target
