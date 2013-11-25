# This .pro file serves a dual purpose:
# 1) invoking gyp through the gyp_qtwebengine script, which in turn makes use of the generated gypi include files
# 2) produce a Makefile that will run ninja, and take care of actually building everything.

TEMPLATE = aux

GYP_ARGS = "-D qt_cross_compile=0"
cross_compile {
    GYP_ARGS = "-D qt_cross_compile=1 -D os_posix=1"
    TOOLCHAIN_SYSROOT = $$[QT_SYSROOT]
    !isEmpty(TOOLCHAIN_SYSROOT): GYP_ARGS += "-D sysroot=\"$${TOOLCHAIN_SYSROOT}\""

    contains(QT_ARCH, "arm") {
        # Extract ARM specific compiler options that we have to pass to gyp
        MARCH = $$extractCFlag("-march=.*")
        MFPU = $$extractCFlag("-mfpu=.*")
        MTUNE = $$extractCFlag("-mtune=.*")
        MFLOAT = $$extractCFlag("-mfloat-abi=.*")
        MARMV = $$replace(MARCH, "armv",)
        MARMV = $$split(MARMV,)
        MARMV = $$member(MARMV, 0)
        MTHUMB = 0
        contains(QMAKE_CFLAGS, "-mthumb"): MTHUMB = 1
        NEON = 0
        contains(MFPU, "neon"): NEON = 1

        GYP_ARGS += "-D target_arch=arm -D arm_version=\"$$MARMV\" -D arm_arch=\"$$MARCH\"" \
                    "-D arm_tune=\"$$MTUNE\" -D arm_fpu=\"$$MFPU\" -D arm_float_abi=\"$$MFLOAT\"" \
                    "-D arm_thumb=\"$$MTHUMB\" -D arm_neon=\"$$NEON\""
    }

    # Needed for v8, see chromium/v8/build/toolchain.gypi
    GYP_ARGS += "-D CXX=\"$$which($$QMAKE_CXX)\""
}

!build_pass {
  message(Running gyp_qtwebengine $${GYP_ARGS}...)
  !system("python ./gyp_qtwebengine $${GYP_ARGS}"): error("-- running gyp_qtwebengine failed --")
}

ninja.target = invoke_ninja
ninja.commands = $$findOrBuildNinja() $$(NINJAFLAGS) -C $$getOutDir()/$$getConfigDir()
ninja.depends: qmake
QMAKE_EXTRA_TARGETS += ninja

build_pass:build_all:default_target.target = all
else: default_target.target = first
default_target.depends = ninja

QMAKE_EXTRA_TARGETS += default_target
