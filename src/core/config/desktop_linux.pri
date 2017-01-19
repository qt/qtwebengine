include(linux.pri)

use?(gn) {
    gn_args += \
        is_clang=false \
        use_sysroot=false \
        use_system_libffi=false

    use?(icecc) {
        gn_args += use_debug_fission=false
    }

} else {

    GYP_ARGS += "-D qt_os=\"desktop_linux\""

    GYP_CONFIG += \
        desktop_linux=1

    clang {
        GYP_CONFIG += werror=
        clang_full_path = $$which($${QMAKE_CXX})
        # Remove the "/bin/clang++" part.
        clang_prefix = $$section(clang_full_path, /, 0, -3)
        GYP_CONFIG += clang=1 host_clang=1 clang_use_chrome_plugins=0 make_clang_dir=$${clang_prefix}
        linux-clang-libc++: GYP_CONFIG += use_system_libcxx=1
    } else {
        GYP_CONFIG += clang=0 host_clang=0
    }
}
