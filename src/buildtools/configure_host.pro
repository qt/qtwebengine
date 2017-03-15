# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

# Pick up the host toolchain
option(host_build)

GN_HOST_CPU = $$gnArch($$QMAKE_HOST.arch)
GN_OS = $$gnOS()

clang: GN_CLANG = true
else: GN_CLANG = false

# We always use the gcc_toolchain, because clang_toolchain is just
# a broken wrapper around it for Google's custom clang binaries.
GN_CONTENTS = \
"import(\"//build/config/sysroot.gni\")" \
"import(\"//build/toolchain/gcc_toolchain.gni\")" \
"gcc_toolchain(\"host\") {" \
"  cc = \"$$which($$QMAKE_CC)\" " \
"  cxx = \"$$which($$QMAKE_CXX)\" " \
"  ld = \"$$which($$QMAKE_LINK)\" " \
"  ar = \"$$which(ar)\" " \
"  nm = \"$$which(nm)\" " \
"  toolchain_args = { " \
"    current_os = \"$$GN_OS\" " \
"    current_cpu = \"$$GN_HOST_CPU\" " \
"  } " \
"}" \
"gcc_toolchain(\"v8_snapshot\") {" \
"  cc = \"$$which($$QMAKE_CC)\" " \
"  cxx = \"$$which($$QMAKE_CXX)\" " \
"  ld = \"$$which($$QMAKE_LINK)\" " \
"  ar = \"$$which(ar)\" " \
"  nm = \"$$which(nm)\" " \
"  toolchain_args = { " \
"    current_os = \"$$GN_OS\" "
# The v8_snapshot toolchain is finished by configure_target.pro

GN_FILE = $$OUT_PWD/../toolchain/BUILD.gn
!build_pass {
    write_file($$GN_FILE, GN_CONTENTS)
}
