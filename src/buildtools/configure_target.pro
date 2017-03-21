# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

GN_CPU = $$gnArch($$QT_ARCH)
GN_OS = $$gnOS()

clang: GN_CLANG = true
else: GN_CLANG = false

GN_V8_HOST_CPU = $$gnArch($$QMAKE_HOST.arch)
contains(GN_CPU, "arm")|contains(GN_CPU, "mips")|contains(GN_CPU, "x86") {
    # The v8 snapshot need a host that matches bitwidth, so we build makesnapshot to 32-bit variants of host.
    contains(GN_V8_HOST_CPU, x64): GN_V8_HOST_CPU = "x86"
    else: contains(GN_V8_HOST_CPU, arm64): GN_V8_HOST_CPU = "arm"
    else: contains(GN_V8_HOST_CPU, mips64): GN_V8_HOST_CPU = "mips"
}

GN_CONTENTS = \
"    current_cpu = \"$$GN_V8_HOST_CPU\" " \
"    v8_current_cpu = \"$$GN_CPU\" " \
"  } " \
"}" \
"gcc_toolchain(\"target\") {" \
"  cc = \"$$which($$QMAKE_CC)\" " \
"  cxx = \"$$which($$QMAKE_CXX)\" " \
"  ld = \"$$which($$QMAKE_LINK)\" " \
"  ar = \"$$which($${CROSS_COMPILE}ar)\" " \
"  nm = \"$$which($${CROSS_COMPILE}nm)\" " \
"  toolchain_args = { " \
"    current_os = \"$$GN_OS\" " \
"    current_cpu = \"$$GN_CPU\" " \
"    is_clang = $$GN_CLANG " \
"  } " \
"}"

GN_FILE = $$OUT_PWD/../toolchain/BUILD.gn
!build_pass {
    write_file($$GN_FILE, GN_CONTENTS, append)
}
