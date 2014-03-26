
CC = $$which($$QMAKE_CC)
ANDROID_TOOLCHAIN = $$dirname(CC)
TOOLCHAIN_SYSROOT = $$ANDROID_BUILD_TOP

GYP_ARGS += "-D qt_os=\"embedded_android\" -D android_src=\"$${TOOLCHAIN_SYSROOT}\" -D android_toolchain=\"$${ANDROID_TOOLCHAIN}\"" \
            "-D android_ndk_root=\"$${TOOLCHAIN_SYSROOT}\" -D android_product_out=\"$${ANDROID_PRODUCT_OUT}\""
